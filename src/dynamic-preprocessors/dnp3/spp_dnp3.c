/*
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
 * Copyright (C) 2011-2012 Sourcefire, Inc.
 *
 * Author: Ryan Jordan
 *
 * Dynamic preprocessor for the DNP3 protocol
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>

#include "sf_types.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_snort_plugin_api.h"
#include "snort_debug.h"
#include "mempool.h"

#include "preprocids.h"
#include "spp_dnp3.h"
#include "sf_preproc_info.h"

#include "dnp3_paf.h"
#include "dnp3_reassembly.h"
#include "dnp3_roptions.h"

#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats dnp3PerfStats;
#endif

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int BUILD_VERSION = 1;
#ifdef SUP_IP6
const char *PREPROC_NAME = "SF_DNP3 (IPV6)";
#else
const char *PREPROC_NAME = "SF_DNP3";
#endif

#define SetupDNP3 DYNAMIC_PREPROC_SETUP

/* Preprocessor config objects */
static tSfPolicyUserContextId dnp3_context_id = NULL;
#ifdef SNORT_RELOAD
static tSfPolicyUserContextId dnp3_swap_context_id = NULL;
#endif
static dnp3_config_t *dnp3_eval_config = NULL;

static MemPool *dnp3_mempool = NULL;


/* Target-based app ID */
#ifdef TARGET_BASED
int16_t dnp3_app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif

/* Prototypes */
static void DNP3Init(char *argp);
static void DNP3OneTimeInit(void);
static dnp3_config_t * DNP3PerPolicyInit(tSfPolicyUserContextId);
static void DNP3RegisterPerPolicyCallbacks(dnp3_config_t *);

static void ProcessDNP3(void *, void *);

#ifdef SNORT_RELOAD
static void DNP3Reload(char *);
static int DNP3ReloadVerify(void);
static void * DNP3ReloadSwap(void);
static void DNP3ReloadSwapFree(void *);
#endif

static void _addPortsToStream5Filter(dnp3_config_t *, tSfPolicyId);
#ifdef TARGET_BASED
static void _addServicesToStream5Filter(tSfPolicyId);
#endif

static void DNP3FreeConfig(tSfPolicyUserContextId context_id);
static void FreeDNP3Data(void *);
static void DNP3CheckConfig(void);
static void DNP3CleanExit(int, void *);

static void ParseDNP3Args(dnp3_config_t *config, char *args);
static void PrintDNP3Config(dnp3_config_t *config);

static int DNP3PortCheck(dnp3_config_t *config, SFSnortPacket *packet);
static MemBucket * DNP3CreateSessionData(SFSnortPacket *);

/* Default memcap is defined as MAX_TCP_SESSIONS * .05 * 20 bytes */
#define DNP3_DEFAULT_MEMCAP (256 * 1024)

/* Register init callback */
void SetupDNP3(void)
{
#ifndef SNORT_RELOAD
    _dpd.registerPreproc("dnp3", DNP3Init);
#else
    _dpd.registerPreproc("dnp3", DNP3Init, DNP3Reload,
                         DNP3ReloadSwap, DNP3ReloadSwapFree);
#endif
}

/* Allocate memory for preprocessor config, parse the args, set up callbacks */
static void DNP3Init(char *argp)
{
    int first_init = 0;
    dnp3_config_t *dnp3_policy = NULL;

    if (dnp3_context_id == NULL)
    {
        first_init = 1;
        DNP3OneTimeInit();
    }

    dnp3_policy = DNP3PerPolicyInit(dnp3_context_id);

    ParseDNP3Args(dnp3_policy, argp);

    PrintDNP3Config(dnp3_policy);

    DNP3RegisterPerPolicyCallbacks(dnp3_policy);
}

static inline void DNP3OneTimeInit()
{
    /* context creation & error checking */
    dnp3_context_id = sfPolicyConfigCreate();
    if (dnp3_context_id == NULL)
    {
        DynamicPreprocessorFatalMessage("Failed to allocate memory for "
                                        "DNP3 config.\n");
    }

    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("SetupDNP3(): The Stream preprocessor "
                                        "must be enabled.\n");
    }

    /* callback registration */
    _dpd.addPreprocConfCheck(DNP3CheckConfig);
    _dpd.addPreprocExit(DNP3CleanExit, NULL, PRIORITY_LAST, PP_DNP3);

#ifdef PERF_PROFILING
    _dpd.addPreprocProfileFunc("dnp3", (void *)&dnp3PerfStats, 0, _dpd.totalPerfStats);
#endif

    /* Set up target-based app id */
#ifdef TARGET_BASED
    dnp3_app_id = _dpd.findProtocolReference("dnp3");
    if (dnp3_app_id == SFTARGET_UNKNOWN_PROTOCOL)
        dnp3_app_id = _dpd.addProtocolReference("dnp3");
#endif
}

/* Responsible for allocating a DNP3 policy. Never returns NULL. */
static inline dnp3_config_t * DNP3PerPolicyInit(tSfPolicyUserContextId context_id)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy();
    dnp3_config_t *dnp3_policy = NULL;

    /* Check for existing policy & bail if found */
    sfPolicyUserPolicySet(context_id, policy_id);
    dnp3_policy = (dnp3_config_t *)sfPolicyUserDataGetCurrent(context_id);
    if (dnp3_policy != NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d): DNP3 preprocessor can only be "
                "configured once.\n", *_dpd.config_file, *_dpd.config_line);
    }

    /* Allocate new policy */
    dnp3_policy = (dnp3_config_t *)calloc(1, sizeof(dnp3_config_t));
    if (!dnp3_policy)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for "
                                        "dnp3 preprocessor configuration.\n");
    }

    sfPolicyUserDataSetCurrent(context_id, dnp3_policy);

    return dnp3_policy;
}

static void DNP3RegisterPerPolicyCallbacks(dnp3_config_t *dnp3_policy)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy();

    /* Callbacks should be avoided if the preproc is disabled. */
    if (dnp3_policy->disabled)
        return;

    _dpd.addPreproc(ProcessDNP3, PRIORITY_APPLICATION, PP_DNP3, PROTO_BIT__TCP|PROTO_BIT__UDP);
    _addPortsToStream5Filter(dnp3_policy, policy_id);
#ifdef TARGET_BASED
    _addServicesToStream5Filter(policy_id);
#endif
    DNP3AddPortsToPaf(dnp3_policy, policy_id);

    _dpd.preprocOptRegister(DNP3_FUNC_NAME, DNP3FuncInit, DNP3FuncEval, free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister(DNP3_OBJ_NAME, DNP3ObjInit, DNP3ObjEval, free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister(DNP3_IND_NAME, DNP3IndInit, DNP3IndEval, free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister(DNP3_DATA_NAME, DNP3DataInit, DNP3DataEval, free, NULL, NULL, NULL, NULL);
}

static void ParseSinglePort(dnp3_config_t *config, char *token)
{
    /* single port number */
    char *endptr;
    unsigned long portnum = _dpd.SnortStrtoul(token, &endptr, 10);

    if ((*endptr != '\0') || (portnum >= MAX_PORTS))
    {
        DynamicPreprocessorFatalMessage("%s(%d): Bad dnp3 port number: %s\n"
                      "Port number must be an integer between 0 and 65535.\n",
                      *_dpd.config_file, *_dpd.config_line, token);
    }

    /* Good port number! */
    config->ports[PORT_INDEX(portnum)] |= CONV_PORT(portnum);
}

static void ParseDNP3Args(dnp3_config_t *config, char *args)
{
    char *saveptr;
    char *token;

    /* Set defaults */
    config->memcap = DNP3_DEFAULT_MEMCAP;
    config->ports[PORT_INDEX(DNP3_PORT)] |= CONV_PORT(DNP3_PORT);
    config->check_crc = 0;

    /* No arguments? Stick with defaults. */
    if (args == NULL)
        return;
    
    token = strtok_r(args, " ,", &saveptr);
    while (token != NULL)
    {
        if (strcmp(token, DNP3_PORTS_KEYWORD) == 0)
        {
            unsigned nPorts = 0;

            /* Un-set the default port */
            config->ports[PORT_INDEX(DNP3_PORT)] = 0;

            /* Parse ports */
            token = strtok_r(NULL, " ,", &saveptr);

            if (token == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d): Missing argument for "
                    "DNP3 preprocessor 'ports' option.\n",
                    *_dpd.config_file, *_dpd.config_line);
            }

            if (isdigit(token[0]))
            {
                ParseSinglePort(config, token);
                nPorts++;
            }
            else if (*token == '{')
            {
                /* list of ports */
                token = strtok_r(NULL, " ,", &saveptr);
                while (token != NULL && *token != '}')
                {
                    ParseSinglePort(config, token);
                    nPorts++;
                    token = strtok_r(NULL, " ,", &saveptr);
                }
            }

            else
            {
                nPorts = 0;
            }
            if ( nPorts == 0 )
            {
                DynamicPreprocessorFatalMessage("%s(%d): Bad DNP3 'ports' argument: '%s'\n"
                              "Argument to DNP3 'ports' must be an integer, or a list "
                              "enclosed in { } braces.\n",
                              *_dpd.config_file, *_dpd.config_line, token);
            }
        }
        else if (strcmp(token, DNP3_MEMCAP_KEYWORD) == 0)
        {
            uint32_t memcap;
            char *endptr;

            /* Parse memcap */
            token = strtok_r(NULL, " ", &saveptr);

            /* In a multiple policy scenario, the memcap from the default policy
               overrides the memcap in any targeted policies. */
            if (_dpd.getParserPolicy() != _dpd.getDefaultPolicy())
            {
                dnp3_config_t *default_config = 
                    (dnp3_config_t *)sfPolicyUserDataGet(dnp3_context_id,
                                                         _dpd.getDefaultPolicy());

                config->memcap = default_config->memcap;
            }
            else
            {
                if (token == NULL)
                {
                    DynamicPreprocessorFatalMessage("%s(%d): Missing argument for DNP3 "
                        "preprocessor 'memcap' option.\n",
                        *_dpd.config_file, *_dpd.config_line);
                }

                memcap = _dpd.SnortStrtoul(token, &endptr, 10);

                if ((token[0] == '-') || (*endptr != '\0') ||
                    (memcap < MIN_DNP3_MEMCAP) || (memcap > MAX_DNP3_MEMCAP))
                {
                    DynamicPreprocessorFatalMessage("%s(%d): Bad DNP3 'memcap' argument: %s\n"
                              "Argument to DNP3 'memcap' must be an integer between "
                              "%d and %d.\n", *_dpd.config_file, *_dpd.config_line,
                              token, MIN_DNP3_MEMCAP, MAX_DNP3_MEMCAP);
                }

                config->memcap = memcap;
            }
        }
        else if (strcmp(token, DNP3_CHECK_CRC_KEYWORD) == 0)
        {
            /* Parse check_crc */
            config->check_crc = 1;
        }
        else if (strcmp(token, DNP3_DISABLED_KEYWORD) == 0)
        {
            /* TODO: if disabled, check that no other stuff is turned on except memcap */
            config->disabled = 1;
        }
        else
        {
            DynamicPreprocessorFatalMessage("%s(%d): Failed to parse dnp3 argument: "
                "%s\n", *_dpd.config_file, *_dpd.config_line,  token);
        }
        token = strtok_r(NULL, " ,", &saveptr);
    }
}

/* Print a DNP3 config */
static void PrintDNP3Config(dnp3_config_t *config)
{
    int index, newline = 1;

    if (config == NULL)
        return;

    _dpd.logMsg("DNP3 config: \n");

    if (config->disabled)
        _dpd.logMsg("    DNP3: INACTIVE\n");

    _dpd.logMsg("    Memcap: %d\n", config->memcap);
    _dpd.logMsg("    Check Link-Layer CRCs: %s\n",
            config->check_crc ?
            "ENABLED":"DISABLED");

    _dpd.logMsg("    Ports:\n");

    /* Loop through port array & print, 5 ports per line */
    for (index = 0; index < MAX_PORTS; index++)
    {
        if (config->ports[PORT_INDEX(index)] & CONV_PORT(index))
        {
            _dpd.logMsg("\t%d", index);
            if ( !((newline++) % 5) )
            {
                _dpd.logMsg("\n");
            }
        }
    }
    _dpd.logMsg("\n");
}

static int DNP3ProcessUDP(dnp3_config_t *dnp3_eval_config,
                          dnp3_session_data_t *sessp, SFSnortPacket *packetp)
{
    /* Possibly multiple PDUs in this UDP payload.
       Split up and process individually. */

    uint16_t bytes_processed = 0;
    int truncated_pdu = 0;

    while (bytes_processed < packetp->payload_size)
    {
        uint8_t *pdu_start;
        uint16_t user_data, num_crcs, pdu_length;
        dnp3_link_header_t *link;

        pdu_start = (uint8_t *)(packetp->payload + bytes_processed);
        link = (dnp3_link_header_t *)pdu_start;

        /* Alert and stop if (a) there's not enough data to read a length, or
           (b) the start bytes are not 0x0564 */
        if ((packetp->payload_size - bytes_processed < (int)sizeof(dnp3_link_header_t)) ||
            (link->start != DNP3_START_BYTES) ||
            (link->len < DNP3_HEADER_REMAINDER_LEN))
        {
            truncated_pdu = 1;
            break;
        }

        /* Calculate the actual length of data to inspect */
        user_data = link->len - DNP3_HEADER_REMAINDER_LEN;
        num_crcs = 1 + (user_data/DNP3_CHUNK_SIZE) + (user_data % DNP3_CHUNK_SIZE? 1 : 0);
        pdu_length = DNP3_MIN_LEN + link->len + (DNP3_CRC_SIZE*num_crcs);

        if (bytes_processed + pdu_length > packetp->payload_size)
        {
            truncated_pdu = 1;
            break;
        }

        DNP3FullReassembly(dnp3_eval_config, sessp, packetp, pdu_start,
                           pdu_length);

        bytes_processed += pdu_length;
    }

    if (truncated_pdu)
    {
        _dpd.alertAdd(GENERATOR_SPP_DNP3, DNP3_DROPPED_FRAME, 1, 0, 3,
                        DNP3_DROPPED_FRAME_STR, 0);
    }

    /* All detection was done when DNP3FullReassembly() called Detect()
       on the reassembled PDUs. Clear the flag to avoid double alerts
       on the last PDU. */
    _dpd.DetectReset((uint8_t *)packetp->payload, packetp->payload_size);

    return DNP3_OK;
}

/* Main runtime entry point */
static void ProcessDNP3(void *ipacketp, void *contextp)
{
    SFSnortPacket *packetp = (SFSnortPacket *)ipacketp;
    MemBucket *tmp_bucket = NULL;
    dnp3_session_data_t *sessp = NULL;
    PROFILE_VARS;

    /* Sanity checks. Should this preprocessor run? */
    if (( !packetp ) ||
        ( !packetp->payload ) ||
        ( !packetp->payload_size ) ||
        ( !IPH_IS_VALID(packetp) ) ||
        ( !packetp->tcp_header && !packetp->udp_header ))
    {
        return;
    }

    /* If TCP, require that PAF flushes full PDUs first. */
    if (packetp->tcp_header && !PacketHasFullPDU(packetp))
        return;

    PREPROC_PROFILE_START(dnp3PerfStats);

    /* When pipelined DNP3 PDUs appear in a single TCP segment or UDP packet,
       the detection engine caches the results of the rule options after
       evaluating on the first PDU. Setting this flag stops the caching. */
    packetp->flags |= FLAG_ALLOW_MULTIPLE_DETECT;

    /* Fetch me a preprocessor config to use with this VLAN/subnet/etc.! */
    dnp3_eval_config = sfPolicyUserDataGetCurrent(dnp3_context_id);

    /* Look for a previously-allocated session data. */
    tmp_bucket = _dpd.streamAPI->get_application_data(packetp->stream_session_ptr, PP_DNP3);
    
    if (tmp_bucket == NULL)
    {
        /* No existing session. Check those ports. */
        if (DNP3PortCheck(dnp3_eval_config, packetp) != DNP3_OK)
        {
            PREPROC_PROFILE_END(dnp3PerfStats);
            return;
        }

        /* Create session data and attach it to the Stream5 session */
        tmp_bucket = DNP3CreateSessionData(packetp);

        if (tmp_bucket == NULL)
        {
            /* Mempool was full, don't process this session. */
            static unsigned int times_mempool_alloc_failed = 0;

            /* Print a message, but only every 1000 times.
               Don't want to flood the log if there's a lot of DNP3 traffic. */
            if (times_mempool_alloc_failed % 1000)
            {
                _dpd.logMsg("WARNING: DNP3 memcap exceeded.\n");
            }
            times_mempool_alloc_failed++;

            PREPROC_PROFILE_END(dnp3PerfStats);
            return;
        }
    }

    sessp = (dnp3_session_data_t *) tmp_bucket->data;

    /* Set reassembly direction */
    if (packetp->flags & FLAG_FROM_CLIENT)
        sessp->direction = DNP3_CLIENT;
    else
        sessp->direction = DNP3_SERVER;

    /* Do preprocessor-specific detection stuff here */
    if (packetp->tcp_header)
    {
        /* Single PDU. PAF already split them up into separate pseudo-packets. */
        DNP3FullReassembly(dnp3_eval_config, sessp, packetp,
                           (uint8_t *)packetp->payload, packetp->payload_size);
    }
    else if (packetp->udp_header)
    {
        DNP3ProcessUDP(dnp3_eval_config, sessp, packetp);
    }

    /* That's the end! */
    PREPROC_PROFILE_END(dnp3PerfStats);
}

/* Check ports & services */
static int DNP3PortCheck(dnp3_config_t *config, SFSnortPacket *packet)
{
#ifdef TARGET_BASED
    int16_t app_id = _dpd.streamAPI->get_application_protocol_id(packet->stream_session_ptr);

    /* call to get_application_protocol_id gave an error */
    if (app_id == SFTARGET_UNKNOWN_PROTOCOL)
        return DNP3_FAIL;

    /* this is positively identified as something non-dnp3 */
    if (app_id && (app_id != dnp3_app_id))
        return DNP3_FAIL;

    /* this is identified as dnp3 */
    if (app_id == dnp3_app_id)
        return DNP3_OK;

    /* fall back to port check */
#endif

    if (config->ports[PORT_INDEX(packet->src_port)] & CONV_PORT(packet->src_port))
        return DNP3_OK;

    if (config->ports[PORT_INDEX(packet->dst_port)] & CONV_PORT(packet->dst_port))
        return DNP3_OK;

    return DNP3_FAIL;
}

static MemBucket * DNP3CreateSessionData(SFSnortPacket *packet)
{
    MemBucket *tmp_bucket = NULL;
    dnp3_session_data_t *data = NULL;

    /* Sanity Check */
    if (!packet || !packet->stream_session_ptr)
        return NULL;

    /* data = (dnp3_session_data_t *)calloc(1, sizeof(dnp3_session_data_t)); */

    tmp_bucket = mempool_alloc(dnp3_mempool);
    if (!tmp_bucket)
        return NULL;

    data = (dnp3_session_data_t *)tmp_bucket->data;

    if (!data)
        return NULL;

    /* Attach to Stream5 session */
    _dpd.streamAPI->set_application_data(packet->stream_session_ptr, PP_DNP3,
        tmp_bucket, FreeDNP3Data);

    /* Not sure when this reference counting stuff got added to the old preprocs */
    data->policy_id = _dpd.getRuntimePolicy();
    data->context_id = dnp3_context_id;
    ((dnp3_config_t *)sfPolicyUserDataGetCurrent(dnp3_context_id))->ref_count++;

    return tmp_bucket;
}


/* Reload functions */
#ifdef SNORT_RELOAD
/* Almost like DNP3Init, but not quite. */
static void DNP3Reload(char *args)
{
    dnp3_config_t *dnp3_policy = NULL;

    if (dnp3_swap_context_id == NULL)
    {
        dnp3_swap_context_id = sfPolicyConfigCreate();
        if (dnp3_swap_context_id == NULL)
        {
            DynamicPreprocessorFatalMessage("Failed to allocate memory "
                                            "for DNP3 config.\n");
        }

        if (_dpd.streamAPI == NULL)
        {
            DynamicPreprocessorFatalMessage("SetupDNP3(): The Stream preprocessor "
                                            "must be enabled.\n");
        }
    }

    dnp3_policy = DNP3PerPolicyInit(dnp3_swap_context_id);

    ParseDNP3Args(dnp3_policy, args);

    PrintDNP3Config(dnp3_policy);

    DNP3RegisterPerPolicyCallbacks(dnp3_policy);

    _dpd.addPreprocReloadVerify(DNP3ReloadVerify);
}

/* Check that Stream5 is still running, and that the memcap didn't change. */
static int DNP3ReloadVerify(void)
{
    dnp3_config_t *current_default_config, *new_default_config;

    if ((dnp3_context_id == NULL) || (dnp3_swap_context_id == NULL))
        return 0;

    current_default_config =
        (dnp3_config_t *)sfPolicyUserDataGet(dnp3_context_id, _dpd.getDefaultPolicy());

    new_default_config =
        (dnp3_config_t *)sfPolicyUserDataGet(dnp3_swap_context_id, _dpd.getDefaultPolicy());

    /* Sanity check. Shouldn't be possible. */
    if (current_default_config == NULL)
        return 0;

    if (new_default_config == NULL)
    {
        DynamicPreprocessorFatalMessage("DNP3 reload: Changing the DNP3 configuration "
            "requires a restart.\n");
    }

    /* Did memcap change? */
    if (current_default_config->memcap != new_default_config->memcap)
    {
        DynamicPreprocessorFatalMessage("DNP3 reload: Changing the DNP3 memcap "
            "requires a restart.\n");
    }

    /* Did stream5 get turned off? */
    if (!_dpd.isPreprocEnabled(PP_STREAM5))
    {
        DynamicPreprocessorFatalMessage("SetupDNP3(): The Stream preprocessor must be enabled.\n");
    }

    return 0;
}

static int DNP3FreeUnusedConfigPolicy(
    tSfPolicyUserContextId context_id,
    tSfPolicyId policy_id,
    void *data
    )
{
    dnp3_config_t *dnp3_config = (dnp3_config_t *)data;

    /* do any housekeeping before freeing dnp3 config */
    if (dnp3_config->ref_count == 0)
    {
        sfPolicyUserDataClear(context_id, policy_id);
        free(dnp3_config);
    }

    return 0;
}

static void * DNP3ReloadSwap(void)
{
    tSfPolicyUserContextId old_context_id = dnp3_context_id;

    if (dnp3_swap_context_id == NULL)
        return NULL;

    dnp3_context_id = dnp3_swap_context_id;
    dnp3_swap_context_id = NULL;

    sfPolicyUserDataIterate(old_context_id, DNP3FreeUnusedConfigPolicy);

    if (sfPolicyUserPolicyGetActive(old_context_id) == 0)
    {
        /* No more outstanding configs - free the config array */
        return (void *)old_context_id;
    }

    return NULL;
}

static void DNP3ReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    DNP3FreeConfig( (tSfPolicyUserContextId)data );
}
#endif

/* Stream5 filter functions */
static void _addPortsToStream5Filter(dnp3_config_t *config, tSfPolicyId policy_id)
{
    if (config == NULL)
        return;

    if (_dpd.streamAPI)
    {
        int portNum;

        for (portNum = 0; portNum < MAX_PORTS; portNum++)
        {
            if(config->ports[(portNum/8)] & (1<<(portNum%8)))
            {
                //Add port the port
                _dpd.streamAPI->set_port_filter_status(
                    IPPROTO_TCP, (uint16_t)portNum, PORT_MONITOR_SESSION, policy_id, 1);
                _dpd.streamAPI->set_port_filter_status(
                    IPPROTO_UDP, (uint16_t)portNum, PORT_MONITOR_SESSION, policy_id, 1);
            }
        }
    }

}

#ifdef TARGET_BASED
static void _addServicesToStream5Filter(tSfPolicyId policy_id)
{
    _dpd.streamAPI->set_service_filter_status(dnp3_app_id, PORT_MONITOR_SESSION, policy_id, 1);
}
#endif

static int DNP3FreeConfigPolicy(
    tSfPolicyUserContextId context_id,
    tSfPolicyId policy_id,
    void *data
    )
{
    dnp3_config_t *dnp3_config = (dnp3_config_t *)data;

    /* do any housekeeping before freeing dnp3_config */

    sfPolicyUserDataClear(context_id, policy_id);
    free(dnp3_config);
    return 0;
}

static void DNP3FreeConfig(tSfPolicyUserContextId context_id)
{
    if (context_id == NULL)
        return;

    sfPolicyUserDataIterate(context_id, DNP3FreeConfigPolicy);
    sfPolicyConfigDelete(context_id);
}

static int DNP3IsEnabled(tSfPolicyUserContextId context_id,
            tSfPolicyId policy_id, void *data)
{
    dnp3_config_t *config = (dnp3_config_t *)data;

    if ((data == NULL) || config->disabled)
        return 0;

    return 1;
}

/* Check an individual policy */
static int DNP3CheckPolicyConfig(
    tSfPolicyUserContextId context_id,
    tSfPolicyId policy_id,
    void *data
    )
{
    dnp3_config_t *config = (dnp3_config_t *)data;

    _dpd.setParserPolicy(policy_id);

    /* In a multiple-policy setting, the preprocessor can be turned on in
       a "disabled" state. In this case, we don't require Stream5. */
    if (config->disabled)
        return 0;

    /* Otherwise, require Stream5. */
    if (!_dpd.isPreprocEnabled(PP_STREAM5))
    {
        DynamicPreprocessorFatalMessage("DNP3CheckPolicyConfig(): "
            "The Stream preprocessor must be enabled.\n");
    }
    return 0;
}

/* Check configs & set up mempool.
   Mempool stuff is in this function because we want to parse & check *ALL*
   of the configs before allocating a mempool. */
static void DNP3CheckConfig(void)
{
    unsigned int max_sessions;

    /* Get default configuration */
    dnp3_config_t *default_config =
        (dnp3_config_t *)sfPolicyUserDataGetDefault(dnp3_context_id);

    /* Check all individual configurations */
    sfPolicyUserDataIterate(dnp3_context_id, DNP3CheckPolicyConfig);

    /* Set up MemPool, but only if a config exists that's not "disabled". */
    if (sfPolicyUserDataIterate(dnp3_context_id, DNP3IsEnabled) == 0)
        return;

    max_sessions = default_config->memcap / sizeof(dnp3_session_data_t);

    dnp3_mempool = (MemPool *)calloc(1, sizeof(MemPool));
    if (mempool_init(dnp3_mempool, max_sessions, sizeof(dnp3_session_data_t)) != 0)
    {
        DynamicPreprocessorFatalMessage("Unable to allocate DNP3 mempool.\n");
    }
}

static void DNP3CleanExit(int signal, void *data)
{
    if (dnp3_context_id != NULL)
    {
        DNP3FreeConfig(dnp3_context_id);
        dnp3_context_id = NULL;
    }

    if ((dnp3_mempool) && (mempool_destroy(dnp3_mempool) == 0))
    {
        free(dnp3_mempool);
        dnp3_mempool = 0;
    }
}

static void FreeDNP3Data(void *bucket)
{
    MemBucket *tmp_bucket = (MemBucket *)bucket;
    dnp3_session_data_t *session;
    dnp3_config_t *config = NULL;

    if ((tmp_bucket == NULL) || (tmp_bucket->data == NULL))
        return;

    session = tmp_bucket->data;

    if (session->context_id != NULL)
    {
        config = (dnp3_config_t *)sfPolicyUserDataGet(session->context_id, session->policy_id);
    }

    if (config != NULL)
    {
        config->ref_count--;
        if ((config->ref_count == 0) &&
            (session->context_id != dnp3_context_id))
        {
            sfPolicyUserDataClear(session->context_id, session->policy_id);
            free(config);

            if (sfPolicyUserPolicyGetActive(session->context_id) == 0)
            {
                /* No more outstanding configs - free the config array */
                DNP3FreeConfig(session->context_id);
            }
        }
    }

    mempool_free(dnp3_mempool, tmp_bucket);
}
