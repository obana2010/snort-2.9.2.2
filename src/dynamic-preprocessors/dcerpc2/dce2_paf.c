/****************************************************************************
 *
 * Copyright (C) 2011-2012 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

#include "sf_types.h"
#include "sfPolicy.h"
#include "sf_dynamic_preprocessor.h"
#include "stream_api.h"
#include "dce2_utils.h"
#include "dce2_session.h"
#include "dce2_smb.h"
#include "snort_dce2.h"
#include "includes/dcerpc.h"
#include "includes/smb.h"

#ifdef ENABLE_PAF

#define DCE2_SMB_PAF_SHIFT(x64, x8) { x64 <<= 8; x64 |= (uint64_t)x8; } 

extern DCE2_ProtoIds dce2_proto_ids;

// Enumerations for PAF states
typedef enum _DCE2_PafSmbStates
{
    DCE2_PAF_SMB_STATES__0 = 0,  // NetBIOS type
    DCE2_PAF_SMB_STATES__1,      // Added bit of NetBIOS length
    DCE2_PAF_SMB_STATES__2,      // First byte of NetBIOS length
    DCE2_PAF_SMB_STATES__3,      // Second byte of NetBIOS length
    // Junk states
    DCE2_PAF_SMB_STATES__4,      // 0xff
    DCE2_PAF_SMB_STATES__5,      // 'S'
    DCE2_PAF_SMB_STATES__6,      // 'M'
    DCE2_PAF_SMB_STATES__7       // 'B'

} DCE2_PafSmbStates;

typedef enum _DCE2_PafTcpStates
{
    DCE2_PAF_TCP_STATES__0 = 0,
    DCE2_PAF_TCP_STATES__1,
    DCE2_PAF_TCP_STATES__2,
    DCE2_PAF_TCP_STATES__3,
    DCE2_PAF_TCP_STATES__4,   // Byte order
    DCE2_PAF_TCP_STATES__5,
    DCE2_PAF_TCP_STATES__6,
    DCE2_PAF_TCP_STATES__7,
    DCE2_PAF_TCP_STATES__8,   // First byte of fragment length
    DCE2_PAF_TCP_STATES__9    // Second byte of fragment length

} DCE2_PafTcpStates;


// State tracker for DCE/RPC over SMB PAF
typedef struct _DCE2_PafSmbState
{
    DCE2_PafSmbStates state;
    uint64_t nb_hdr;   // Enough for NetBIOS header and 4 bytes SMB header

} DCE2_PafSmbState;

// State tracker for DCE/RPC over TCP PAF
typedef struct _DCE2_PafTcpState
{
    DCE2_PafTcpStates state;
    DceRpcBoFlag byte_order;
    uint16_t frag_len;

} DCE2_PafTcpState;


// Local function prototypes
static inline bool DCE2_PafSmbIsValidNetbiosHdr(uint32_t, bool);
static inline bool DCE2_PafAbort(void *, uint32_t);
static PAF_Status DCE2_SmbPaf(void *, void **, const uint8_t *, uint32_t, uint32_t, uint32_t *);
static PAF_Status DCE2_TcpPaf(void *, void **, const uint8_t *, uint32_t, uint32_t, uint32_t *);


/*********************************************************************
 * Function: DCE2_PafAbort()
 *
 * Purpose: Queries the dcerpc2 session data to see if paf abort
 *          flag is set.
 *
 * Arguments:
 *  void *   - stream session pointer
 *  uint32_t - flags passed in to callback.
 *             Should have PKT_FROM_CLIENT or PKT_FROM_SERVER set.
 *
 * Returns:
 *  bool - true if missed packets, false if not
 *
 *********************************************************************/
static inline bool DCE2_PafAbort(void *ssn, uint32_t flags)
{
    DCE2_SsnData *sd = (DCE2_SsnData *)_dpd.streamAPI->get_application_data(ssn, PP_DCE2);

    if (sd != NULL)
    {
        if (DCE2_SsnPafAbort(sd))
            return true;
        else if (!DCE2_SsnSeenClient(sd) && (flags & FLAG_FROM_SERVER))
            return true;
    }

    return false;
}

/*********************************************************************
 * Function: DCE2_PafSmbIsValidNetbiosHdr()
 *
 * Purpose: Validates that the NetBIOS header is valid.  If in
 *          junk states, header type must be Session Message.
 *
 * Arguments:
 *  uint32_t - the 4 bytes of the NetBIOS header
 *  bool - whether we're in a junk data state or not
 *
 * Returns:
 *  bool - true if valid, false if not
 *
 *********************************************************************/
static inline bool DCE2_PafSmbIsValidNetbiosHdr(uint32_t nb_hdr, bool junk)
{
    uint8_t type = (uint8_t)(nb_hdr >> 24);
    uint8_t bit = (uint8_t)((nb_hdr & 0x00ff0000) >> 16);

    if (junk)
    {
        if (type != NBSS_SESSION_TYPE__MESSAGE)
            return false;
    }
    else
    {
        switch (type)
        {
            case NBSS_SESSION_TYPE__MESSAGE:
            case NBSS_SESSION_TYPE__REQUEST:
            case NBSS_SESSION_TYPE__POS_RESPONSE:
            case NBSS_SESSION_TYPE__NEG_RESPONSE:
            case NBSS_SESSION_TYPE__RETARGET_RESPONSE:
            case NBSS_SESSION_TYPE__KEEP_ALIVE:
                break;
            default:
                return false;
        }
    }

    if ((bit != 0x00) && (bit != 0x01))
        return false;
    
    return true;
}

/*********************************************************************
 * Function: DCE2_SmbPaf()
 *
 * Purpose: The DCE/RPC over SMB PAF callback.
 *          Inspects a byte at a time changing state and shifting
 *          bytes onto the 64bit nb_hdr member.  At state 3
 *          determines if NetBIOS header is valid and if so sets
 *          flush point.  If not valid goes to states 4-7 where
 *          there is the possibility that junk data was inserted
 *          before request/response.  Needs to validate SMB ID at
 *          this point.  At state 7 determines if NetBIOS header
 *          is valid and that the SMB ID is present.  Stays in
 *          state 7 until this is the case.
 *
 * Arguments:
 *  void * - stream5 session pointer
 *  void ** - SMB state tracking structure
 *  const uint8_t * - payload data to inspect
 *  uint32_t - length of payload data
 *  uint32_t - flags to check whether client or server
 *  uint32_t * - pointer to set flush point
 *
 * Returns:
 *  PAF_Status - PAF_FLUSH if flush point found, PAF_SEARCH otherwise
 *
 *********************************************************************/
PAF_Status DCE2_SmbPaf(void *ssn, void **user, const uint8_t *data,
        uint32_t len, uint32_t flags, uint32_t *fp)
{
    DCE2_PafSmbState *ss = *(DCE2_PafSmbState **)user;
    uint32_t n = 0;
    PAF_Status ps = PAF_SEARCH;
    uint32_t nb_hdr;
    uint32_t nb_len;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "\nIn DCE2_SmbPaf: %u bytes of data\n", len));

#ifdef DEBUG_MSGS
    if (flags & FLAG_FROM_CLIENT)
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Client\n"));
    else
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Server\n"));
#endif

    if (ss == NULL)
    {
        // beware - we allocate here but s5 calls free() directly
        // so no pointers allowed
        ss = calloc(1, sizeof(DCE2_PafSmbState));

        if (ss == NULL)
            return PAF_ABORT;

        *user = ss;
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Start state: %u\n", ss->state));

    if (DCE2_PafAbort(ssn, flags))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Aborting PAF\n"));
        return PAF_ABORT;
    }

    while (n < len)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "data[n]: 0x%02x", data[n]));

#ifdef DEBUG_MSGS
        if (isprint(data[n]))
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, " : %c\n", data[n]));
        else
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "\n"));
#endif

        switch (ss->state)
        {
            case DCE2_PAF_SMB_STATES__0:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "State 0\n"));
                ss->nb_hdr = (uint64_t)data[n];
                ss->state++;
                break;
            case DCE2_PAF_SMB_STATES__3:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "State 3\n"));
                DCE2_SMB_PAF_SHIFT(ss->nb_hdr, data[n]);
                if (DCE2_PafSmbIsValidNetbiosHdr((uint32_t)ss->nb_hdr, false))
                {
                    nb_hdr = htonl((uint32_t)ss->nb_hdr);
                    nb_len = NbssLen((const NbssHdr *)&nb_hdr);
                    *fp = (nb_len + sizeof(NbssHdr) + n) - ss->state;
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Setting flush "
                                             "point for non-junk data: %u\n\n", *fp));
                    ss->state = DCE2_PAF_SMB_STATES__0;
                    return PAF_FLUSH;
                }
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Invalid NetBIOS header - "
                                         "entering junk data states.\n"));
                ss->state++;
                break;
            case DCE2_PAF_SMB_STATES__7:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "State 7\n"));
                DCE2_SMB_PAF_SHIFT(ss->nb_hdr, data[n]);

                if (!DCE2_PafSmbIsValidNetbiosHdr((uint32_t)(ss->nb_hdr >> 32), true))
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Invalid NetBIOS header - "
                                             "staying in State 7.\n"));
                    break;
                }
                if (((uint32_t)ss->nb_hdr != DCE2_SMB_ID)
                        && ((uint32_t)ss->nb_hdr != DCE2_SMB2_ID))
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Invalid SMB ID - "
                                             "staying in State 7.\n"));
                    break;
                }

                nb_hdr = htonl((uint32_t)(ss->nb_hdr >> 32));
                nb_len = NbssLen((const NbssHdr *)&nb_hdr);
                *fp = (nb_len + sizeof(NbssHdr) + n) - ss->state;
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Setting flush point "
                                         "for junk data: %u\n\n", *fp));
                ss->state = DCE2_PAF_SMB_STATES__0;
                return PAF_FLUSH;
            default:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "State %u\n", ss->state));
                DCE2_SMB_PAF_SHIFT(ss->nb_hdr, data[n]);
                ss->state++;
                break;
        }

        n++;
    }

    return ps;
}

/*********************************************************************
 * Function: DCE2_TcpPaf()
 *
 * Purpose: The DCE/RPC over TCP PAF callback.
 *          Inspects a byte at a time changing state.  At state 4
 *          gets byte order of PDU.  At states 8 and 9 gets
 *          fragment length and sets flush point if no more data.
 *          Otherwise accumulates flush points because there can
 *          be multiple PDUs in a single TCP segment (evasion case).
 *
 * Arguments:
 *  void * - stream5 session pointer
 *  void ** - TCP state tracking structure
 *  const uint8_t * - payload data to inspect
 *  uint32_t - length of payload data
 *  uint32_t - flags to check whether client or server
 *  uint32_t * - pointer to set flush point
 *
 * Returns:
 *  PAF_Status - PAF_FLUSH if flush point found, PAF_SEARCH otherwise
 *
 *********************************************************************/
PAF_Status DCE2_TcpPaf(void *ssn, void **user, const uint8_t *data,
        uint32_t len, uint32_t flags, uint32_t *fp)
{
    DCE2_PafTcpState *ds = *(DCE2_PafTcpState **)user;
    uint32_t n = 0;
    int start_state;
    PAF_Status ps = PAF_SEARCH;
    uint32_t tmp_fp = 0;
    DCE2_SsnData *sd = (DCE2_SsnData *)_dpd.streamAPI->get_application_data(ssn, PP_DCE2);
    int num_requests = 0;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "\nIn DCE2_TcpPaf: %u bytes of data\n", len));

    if (sd == NULL)
    {
        // Need packet to see if it's an autodetect port then do an autodetect
        // if autodetect port and not autodetected
        //     return PAF_ABORT

        bool cont = false;

#ifdef TARGET_BASED
        if (_dpd.isAdaptiveConfigured(_dpd.getRuntimePolicy(), 0))
        {
            int16_t proto_id = _dpd.streamAPI->get_application_protocol_id(ssn);

            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "No session data - checking adaptive "
                                     "to see if it's DCE/RPC.\n"));

            if (proto_id == dce2_proto_ids.dcerpc)
                cont = true;
        }
        else
        {
#endif
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "No session data - autodetecting\n"));

            if (len >= sizeof(DceRpcCoHdr))
            {
                DceRpcCoHdr *co_hdr = (DceRpcCoHdr *)data;

                if ((DceRpcCoVersMaj(co_hdr) == DCERPC_PROTO_MAJOR_VERS__5) &&
                    (DceRpcCoVersMin(co_hdr) == DCERPC_PROTO_MINOR_VERS__0) &&
                    ((DceRpcCoPduType(co_hdr) == DCERPC_PDU_TYPE__BIND) ||
                     (DceRpcCoPduType(co_hdr) == DCERPC_PDU_TYPE__BIND_ACK)) &&
                    (DceRpcCoFragLen(co_hdr) >= sizeof(DceRpcCoHdr)))
                {
                    cont = true;
                }
            }
            else if ((*data == DCERPC_PROTO_MAJOR_VERS__5) && (flags & FLAG_FROM_CLIENT))
            {
                cont = true;
            }
#ifdef TARGET_BASED
        }
#endif

        if (!cont)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Couldn't autodetect - aborting\n"));
            return PAF_ABORT;
        }

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Autodetected!\n"));
    }

    if (ds == NULL)
    {
        // beware - we allocate here but s5 calls free() directly
        // so no pointers allowed
        ds = calloc(1, sizeof(DCE2_PafTcpState));

        if (ds == NULL)
            return PAF_ABORT;

        *user = ds;
    }

#ifdef DEBUG_MSGS
    if (flags & FLAG_FROM_CLIENT)
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Client\n"));
    else
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Server\n"));
#endif

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Start state: %u\n", ds->state));
    start_state = (uint8_t)ds->state;  // determines how many bytes already looked at

    if (DCE2_PafAbort(ssn, flags))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Aborting PAF\n"));
        return PAF_ABORT;
    }

    while (n < len)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "data[n]: 0x%02x", data[n]));

#ifdef DEBUG_MSGS
        if (isprint(data[n]))
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, " : %c\n", data[n]));
        else
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "\n"));
#endif

        switch (ds->state)
        {
            case DCE2_PAF_TCP_STATES__4:  // Get byte order
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "State 4\n"));
                ds->byte_order = DceRpcByteOrder(data[n]);
                ds->state++;
#ifdef DEBUG_MSGS
                if (ds->byte_order == DCERPC_BO_FLAG__LITTLE_ENDIAN)
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Got byte order: Little endian\n"));
                else
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Got byte order: Big endian\n"));
#endif
                break;
            case DCE2_PAF_TCP_STATES__8:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "State 8\n"));
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Getting first byte of frag length\n"));
                if (ds->byte_order == DCERPC_BO_FLAG__LITTLE_ENDIAN)
                    ds->frag_len = data[n];
                else
                    ds->frag_len = data[n] << 8;
                ds->state++;
                break;
            case DCE2_PAF_TCP_STATES__9:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "State 9\n"));
                if (ds->byte_order == DCERPC_BO_FLAG__LITTLE_ENDIAN)
                    ds->frag_len |= data[n] << 8;
                else
                    ds->frag_len |= data[n];
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Getting second byte of frag length\n"));

                /* If we get a bad frag length abort */
                if (ds->frag_len < sizeof(DceRpcCoHdr))
                    return PAF_ABORT;

                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Got frag_len: %u\n", ds->frag_len));

                /* Increment n here so we can continue */
                n += ds->frag_len - (uint8_t)ds->state;
                num_requests++;
                /* Might have multiple PDUs in one segment.  If the last PDU is partial,
                 * flush just before it */
                if ((num_requests == 1) || (n <= len))
                    tmp_fp += ds->frag_len;
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Requests: %u\n", num_requests));
                ds->state = DCE2_PAF_TCP_STATES__0;
                continue;  // we incremented n already
            default:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "State %u\n", ds->state));
                ds->state++;
                break;
        }

        n++;
    }

    if (tmp_fp != 0)
    {
        *fp = tmp_fp - start_state;
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Setting flush point: %u\n", *fp));
        return PAF_FLUSH;
    }

    return ps;
}

#endif  // ENABLE_PAF

/*********************************************************************
 * Function: DCE2_PafRegister()
 *
 * Purpose: Registers callbacks for interested ports.  SMB and TCP
 *          ports are mutually exclusive so only one or the other
 *          will be registered for any given port.
 *
 * Arguments:
 *  uint16_t - port to register
 *  tSfPolicyId - the policy to register for
 *  DCE2_TransType - the type of DCE/RPC transport to register for.
 *
 * Returns:
 *  int - 0 for success.
 *
 *********************************************************************/
int DCE2_PafRegister(uint16_t port, tSfPolicyId pid, DCE2_TransType trans)
{
#ifdef ENABLE_PAF
    if (!_dpd.isPafEnabled())
        return 0;

    //DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
    //            "%s: policy %u, port %u\n", __FUNCTION__, pid, port););

    switch (trans)
    {
        case DCE2_TRANS_TYPE__SMB:
            _dpd.streamAPI->register_paf_cb(pid, port, 0, DCE2_SmbPaf, true);
            _dpd.streamAPI->register_paf_cb(pid, port, 1, DCE2_SmbPaf, true);
            break;
        case DCE2_TRANS_TYPE__TCP:
            _dpd.streamAPI->register_paf_cb(pid, port, 0, DCE2_TcpPaf, true);
            _dpd.streamAPI->register_paf_cb(pid, port, 1, DCE2_TcpPaf, true);
            break;
        default:
            DCE2_Die("Invalid transport type sent to paf registration function");
            break;
    }
#endif

    return 0;
}

