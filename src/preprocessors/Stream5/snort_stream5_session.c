/* $Id$ */

/*
** Copyright (C) 2005-2012 Sourcefire, Inc.
** AUTHOR: Steven Sturges <ssturges@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* snort_stream5_session.c
 *
 * Purpose: Hash Table implementation of session management functions for
 *          Stream5 preprocessor.
 *
 * Arguments:
 *
 * Effect:
 *
 * Comments:
 *
 * Any comments?
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "decode.h"
#include "sf_types.h"
#include "snort_debug.h"
#include "log.h"
#include "util.h"
#include "snort_stream5_session.h"
#include "stream5_common.h"
#include "sfhashfcn.h"
#include "bitop_funcs.h"

#ifndef WIN32
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif

#include "snort.h"

#if 0
// if you want to use this, print ip_l,h for proper ip4,6 support
void PrintSessionKey(SessionKey *skey)
{
    LogMessage("SessionKey:\n");
    LogMessage("    ip_l     = 0x%08X\n", skey->ip_l);
    LogMessage("    ip_h     = 0x%08X\n", skey->ip_h);
    LogMessage("    prt_l    = %d\n", skey->port_l);
    LogMessage("    prt_h    = %d\n", skey->port_h);
    LogMessage("    vlan_tag = %d\n", skey->vlan_tag);
#ifdef MPLS
    LogMessage("  mpls label = 0x%08X\n", skey->mplsLabel);
#endif
}
#endif

int GetLWSessionCount(Stream5SessionCache *sessionCache)
{
    if (sessionCache && sessionCache->hashTable)
        return sessionCache->hashTable->count;
    else
        return 0;
}

int GetLWSessionKeyFromIpPort(
                    snort_ip_p srcIP,
                    uint16_t srcPort,
                    snort_ip_p dstIP,
                    uint16_t dstPort,
                    char proto,
                    uint16_t vlan,
                    uint32_t mplsId,
                    SessionKey *key)
{
    uint16_t sport;
    uint16_t dport;
    /* Because the key is going to be used for hash lookups,
     * the lower of the values of the IP address field is
     * stored in the key->ip_l and the port for that ip is
     * stored in key->port_l.
     */

    if (!key)
        return 0;

#ifdef SUP_IP6
    if (IS_IP4(srcIP))
    {
        uint32_t *src;
        uint32_t *dst;

        switch (proto)
        {
            case IPPROTO_TCP:
            case IPPROTO_UDP:
                sport = srcPort;
                dport = dstPort;
                break;
            case IPPROTO_ICMP:
            default:
                sport = dport = 0;
                break;
        }

        src = srcIP->ip32;
        dst = dstIP->ip32;

        /* These comparisons are done in this fashion for performance reasons */
        if (*src < *dst)
        {
            COPY4(key->ip_l, src);
            COPY4(key->ip_h, dst);
            key->port_l = sport;
            key->port_h = dport;
        }
        else if (*src == *dst)
        {
            COPY4(key->ip_l, src);
            COPY4(key->ip_h, dst);
            if (sport < dport)
            {
                key->port_l = sport;
                key->port_h = dport;
            }
            else
            {
                key->port_l = dport;
                key->port_h = sport;
            }
        }
        else
        {
            COPY4(key->ip_l, dst);
            key->port_l = dport;
            COPY4(key->ip_h, src);
            key->port_h = sport;
        }
# ifdef MPLS
        if (ScMplsOverlappingIp() &&
            isPrivateIP(*src) && isPrivateIP(*dst))
        {
            key->mplsLabel = mplsId;
        }
        else
        {
        	key->mplsLabel = 0;
        }
# endif
    }
    else
    {
        /* IPv6 */
        sfip_t *src;
        sfip_t *dst;

        switch (proto)
        {
            case IPPROTO_TCP:
            case IPPROTO_UDP:
                sport = srcPort;
                dport = dstPort;
                break;
            case IPPROTO_ICMP:
            default:
                sport = dport = 0;
                break;
        }

        src = srcIP;
        dst = dstIP;

        if (sfip_fast_lt6(src, dst))
        {
            COPY4(key->ip_l, src->ip32);
            key->port_l = sport;
            COPY4(key->ip_h, dst->ip32);
            key->port_h = dport;
        }
        else if (sfip_fast_eq6(src, dst))
        {
            COPY4(key->ip_l, src->ip32);
            COPY4(key->ip_h, dst->ip32);
            if (sport < dport)
            {
                key->port_l = sport;
                key->port_h = dport;
            }
            else
            {
                key->port_l = dport;
                key->port_h = sport;
            }
        }
        else
        {
            COPY4(key->ip_l, dst->ip32);
            key->port_l = dport;
            COPY4(key->ip_h, src->ip32);
            key->port_h = sport;
        }
# ifdef MPLS
        if (ScMplsOverlappingIp())
        {
            key->mplsLabel = mplsId;
        }
        else
        {
        	key->mplsLabel = 0;
        }
# endif
    }
#else
    switch (proto)
    {
        case IPPROTO_TCP:
        case IPPROTO_UDP:
            sport = srcPort;
            dport = dstPort;
            break;
        case IPPROTO_ICMP:
        default:
            sport = dport = 0;
            break;
    }

    /* These comparisons are done in this fashion for performance reasons */
    if (IP_LESSER(srcIP, dstIP))
    {
        IP_COPY_VALUE(key->ip_l, srcIP);
        key->port_l = sport;
        IP_COPY_VALUE(key->ip_h, dstIP);
        key->port_h = dport;
    }
    else if (IP_EQUALITY(srcIP, dstIP))
    {
        IP_COPY_VALUE(key->ip_l, srcIP);
        IP_COPY_VALUE(key->ip_h, dstIP);
        if (sport < dport)
        {
            key->port_l = sport;
            key->port_h = dport;
        }
        else
        {
            key->port_l = dport;
            key->port_h = sport;
        }
    }
    else
    {
        IP_COPY_VALUE(key->ip_l, dstIP);
        key->port_l = dport;
        IP_COPY_VALUE(key->ip_h, srcIP);
        key->port_h = sport;
    }
# ifdef MPLS
    if (ScMplsOverlappingIp() &&
        isPrivateIP(key->ip_l) && isPrivateIP(key->ip_h))
    {
        key->mplsLabel = mplsId;
    }
    else
    {
    	key->mplsLabel = 0;
    }
# endif
#endif

    key->protocol = proto;

    if (!ScVlanAgnostic())
        key->vlan_tag = vlan;
    else
        key->vlan_tag = 0;

    key->pad = 0;
#ifdef MPLS
    key->mplsPad = 0;
#endif
    return 1;
}

int GetLWSessionKey(Packet *p, SessionKey *key)
{
    char proto = GET_IPH_PROTO(p);
    uint32_t mplsId = 0;
    uint16_t vlanId = 0;
# ifdef MPLS
    if (ScMplsOverlappingIp() && (p->mpls != NULL))
    {
        mplsId = p->mplsHdr.label;
    }
#endif
    if (p->vh && !ScVlanAgnostic())
        vlanId = (uint16_t)VTH_VLAN(p->vh);
    return GetLWSessionKeyFromIpPort(GET_SRC_IP(p), p->sp,
        GET_DST_IP(p), p->dp,
        proto, vlanId, mplsId, key);
}

void GetLWPacketDirection(Packet *p, Stream5LWSession *ssn)
{
#ifndef SUP_IP6
    if (p->iph->ip_src.s_addr == ssn->client_ip)
    {
        if ( IsTCP(p) )
        {
            if (p->tcph->th_sport == ssn->client_port)
            {
                p->packet_flags |= PKT_FROM_CLIENT;
            }
            else
            {
                p->packet_flags |= PKT_FROM_SERVER;
            }
        }
        else if ( IsUDP(p) )
        {
            if (p->udph->uh_sport == ssn->client_port)
            {
                p->packet_flags |= PKT_FROM_CLIENT;
            }
            else
            {
                p->packet_flags |= PKT_FROM_SERVER;
            }
        }
        else
        {
            p->packet_flags |= PKT_FROM_CLIENT;
        }
    }
    else if (p->iph->ip_dst.s_addr == ssn->client_ip)
    {
        if ( IsTCP(p) )
        {
            if (p->tcph->th_dport == ssn->client_port)
            {
                p->packet_flags |= PKT_FROM_SERVER;
            }
            else
            {
                p->packet_flags |= PKT_FROM_CLIENT;
            }
        }
        else if ( IsUDP(p) )
        {
            if (p->udph->uh_dport == ssn->client_port)
            {
                p->packet_flags |= PKT_FROM_SERVER;
            }
            else
            {
                p->packet_flags |= PKT_FROM_CLIENT;
            }
        }
        else
        {
            p->packet_flags |= PKT_FROM_SERVER;
        }
    }
    else
    {
        /* Uh, no match of the packet to the session. */
        /* Probably should log an error */
    }
#else
    if(IS_IP4(p))
    {
        if (sfip_fast_eq4(&p->ip4h->ip_src, &ssn->client_ip))
        {
            if (GET_IPH_PROTO(p) == IPPROTO_TCP)
            {
                if (p->tcph->th_sport == ssn->client_port)
                {
                    p->packet_flags |= PKT_FROM_CLIENT;
                }
                else
                {
                    p->packet_flags |= PKT_FROM_SERVER;
                }
            }
            else if (GET_IPH_PROTO(p) == IPPROTO_UDP)
            {
                if (p->udph->uh_sport == ssn->client_port)
                {
                    p->packet_flags |= PKT_FROM_CLIENT;
                }
                else
                {
                    p->packet_flags |= PKT_FROM_SERVER;
                }
            }
            else
            {
                p->packet_flags |= PKT_FROM_CLIENT;
            }
        }
        else if (sfip_fast_eq4(&p->ip4h->ip_dst, &ssn->client_ip))
        {
            if  (GET_IPH_PROTO(p) == IPPROTO_TCP)
            {
                if (p->tcph->th_dport == ssn->client_port)
                {
                    p->packet_flags |= PKT_FROM_SERVER;
                }
                else
                {
                    p->packet_flags |= PKT_FROM_CLIENT;
                }
            }
            else if (GET_IPH_PROTO(p) == IPPROTO_UDP)
            {
                if (p->udph->uh_dport == ssn->client_port)
                {
                    p->packet_flags |= PKT_FROM_SERVER;
                }
                else
                {
                    p->packet_flags |= PKT_FROM_CLIENT;
                }
            }
            else
            {
                p->packet_flags |= PKT_FROM_SERVER;
            }
        }
    }
    else /* IS_IP6(p) */
    {
        if (sfip_fast_eq6(&p->ip6h->ip_src, &ssn->client_ip))
        {
            if (GET_IPH_PROTO(p) == IPPROTO_TCP)
            {
                if (p->tcph->th_sport == ssn->client_port)
                {
                    p->packet_flags |= PKT_FROM_CLIENT;
                }
                else
                {
                    p->packet_flags |= PKT_FROM_SERVER;
                }
            }
            else if (GET_IPH_PROTO(p) == IPPROTO_UDP)
            {
                if (p->udph->uh_sport == ssn->client_port)
                {
                    p->packet_flags |= PKT_FROM_CLIENT;
                }
                else
                {
                    p->packet_flags |= PKT_FROM_SERVER;
                }
            }
            else
            {
                p->packet_flags |= PKT_FROM_CLIENT;
            }
        }
        else if (sfip_fast_eq6(&p->ip6h->ip_dst, &ssn->client_ip))
        {
            if  (GET_IPH_PROTO(p) == IPPROTO_TCP)
            {
                if (p->tcph->th_dport == ssn->client_port)
                {
                    p->packet_flags |= PKT_FROM_SERVER;
                }
                else
                {
                    p->packet_flags |= PKT_FROM_CLIENT;
                }
            }
            else if (GET_IPH_PROTO(p) == IPPROTO_UDP)
            {
                if (p->udph->uh_dport == ssn->client_port)
                {
                    p->packet_flags |= PKT_FROM_SERVER;
                }
                else
                {
                    p->packet_flags |= PKT_FROM_CLIENT;
                }
            }
            else
            {
                p->packet_flags |= PKT_FROM_SERVER;
            }
        }
    }
#endif /* SUP_IP6 */
}

Stream5LWSession *GetLWSession(Stream5SessionCache *sessionCache, Packet *p, SessionKey *key)
{
    Stream5LWSession *returned = NULL;
    SFXHASH_NODE *hnode;

    if (!sessionCache)
        return NULL;

    if (!GetLWSessionKey(p, key))
    {
        return NULL;
    }

    hnode = sfxhash_find_node(sessionCache->hashTable, key);

    if (hnode && hnode->data)
    {
        /* This is a unique hnode, since the sfxhash finds the
         * same key before returning this node.
         */
        returned = (Stream5LWSession *)hnode->data;
        if (returned && (returned->last_data_seen < p->pkth->ts.tv_sec))
        {
            returned->last_data_seen = p->pkth->ts.tv_sec;
        }
    }
    return returned;
}

Stream5LWSession *GetLWSessionFromKey(Stream5SessionCache *sessionCache, SessionKey *key)
{
    Stream5LWSession *returned = NULL;
    SFXHASH_NODE *hnode;

    if (!sessionCache)
        return NULL;

    hnode = sfxhash_find_node(sessionCache->hashTable, key);

    if (hnode && hnode->data)
    {
        /* This is a unique hnode, since the sfxhash finds the
         * same key before returning this node.
         */
        returned = (Stream5LWSession *)hnode->data;
    }
    return returned;
}

void FreeLWApplicationData(Stream5LWSession *ssn)
{
    Stream5AppData *tmpData, *appData = ssn->appDataList;
    while (appData)
    {
        if (appData->freeFunc && appData->dataPointer)
        {
            appData->freeFunc(appData->dataPointer);
        }

        tmpData = appData->next;
        free(appData);
        appData = tmpData;
    }

    ssn->appDataList = NULL;
}

int RemoveLWSession(Stream5SessionCache *sessionCache, Stream5LWSession *ssn)
{
    Stream5Config *pPolicyConfig = NULL;
    tSfPolicyId policy_id = ssn->policy_id;
    mempool_free(&s5FlowMempool, ssn->flowdata);
    ssn->flowdata = NULL;

    pPolicyConfig = (Stream5Config *)sfPolicyUserDataGet(ssn->config, policy_id);

    if (pPolicyConfig != NULL)
    {
        pPolicyConfig->ref_count--;
        if ((pPolicyConfig->ref_count == 0) && (ssn->config != s5_config))
        {
            sfPolicyUserDataClear (ssn->config, policy_id);
            Stream5FreeConfig(pPolicyConfig);

            if (sfPolicyUserPolicyGetActive(ssn->config) == 0)
                Stream5FreeConfigs(ssn->config);
        }
    }

    return sfxhash_remove(sessionCache->hashTable, &(ssn->key));
}

int DeleteLWSession(Stream5SessionCache *sessionCache,
                    Stream5LWSession *ssn, char *delete_reason)
{
    /* Need to save the current configurations being used since this function
     * may cause a packet reassembly on this deleted session when flushing
     * (via sessionCache->cleanup_fcn) and a preprocessor may call an API
     * function changing the configurations to this one to be deleted */
    Stream5GlobalConfig *save_global_eval_config = s5_global_eval_config;
    Stream5TcpConfig *save_tcp_eval_config = s5_tcp_eval_config;
    Stream5UdpConfig *save_udp_eval_config = s5_udp_eval_config;
    Stream5IcmpConfig *save_icmp_eval_config = s5_icmp_eval_config;

    int ret;
    uint32_t prune_log_max;

    /* Save the current mem in use before pruning */
    uint32_t old_mem_in_use = mem_in_use;

    /* And save some info on that session */
#ifdef SUP_IP6
    sfip_t client_ip;
    sfip_t server_ip;
#else
    struct in_addr client_ip;
    struct in_addr server_ip;
#endif
    uint16_t client_port = ntohs(ssn->client_port);
    uint16_t server_port = ntohs(ssn->server_port);
    uint16_t lw_session_state = ssn->session_state;
    uint32_t lw_session_flags = ssn->session_flags;
#ifdef TARGET_BASED
    int16_t app_proto_id = ssn->application_protocol;
#endif

#ifdef SUP_IP6
    sfip_set_ip(&client_ip, &ssn->client_ip);
    sfip_set_ip(&server_ip, &ssn->server_ip);
#else
    client_ip.s_addr = ssn->client_ip;
    server_ip.s_addr = ssn->server_ip;
#endif

    /*
     * Call callback to cleanup the protocol (TCP/UDP/ICMP)
     * specific session details
     */
    if (sessionCache->cleanup_fcn)
        sessionCache->cleanup_fcn(ssn);

    FreeLWApplicationData(ssn);

    /* Need to save this off since the global config might be from an
     * older session - because of a reload - and that config might
     * get freed after removing the session */
    prune_log_max = s5_global_eval_config->prune_log_max;

    ret = RemoveLWSession(sessionCache, ssn);

    /* If we're pruning and we clobbered some large amount, log a
     * message about that session. */
    if (prune_log_max
            && ((old_mem_in_use - mem_in_use ) > prune_log_max))
    {
        char *client_ip_str, *server_ip_str;
#ifdef SUP_IP6
        client_ip_str = SnortStrdup(inet_ntoa(&client_ip));
        server_ip_str = SnortStrdup(inet_ntoa(&server_ip));
#else
        client_ip_str = SnortStrdup(inet_ntoa(client_ip));
        server_ip_str = SnortStrdup(inet_ntoa(server_ip));
#endif
        LogMessage("S5: Pruned session from cache that was "
                   "using %d bytes (%s). %s %d --> %s %d "
#ifdef TARGET_BASED
                   "(%d) "
#endif
                   ": LWstate 0x%x LWFlags 0x%x\n",
                   old_mem_in_use - mem_in_use,
                   delete_reason,
                   client_ip_str, client_port,
                   server_ip_str, server_port,
#ifdef TARGET_BASED
                   app_proto_id,
#endif
                   lw_session_state, lw_session_flags);
        free(client_ip_str);
        free(server_ip_str);
    }

    s5_global_eval_config = save_global_eval_config;
    s5_tcp_eval_config = save_tcp_eval_config;
    s5_udp_eval_config = save_udp_eval_config;
    s5_icmp_eval_config = save_icmp_eval_config;

    return ret;
}

int PurgeLWSessionCache(Stream5SessionCache *sessionCache)
{
    int retCount = 0;
    Stream5LWSession *idx;
    SFXHASH_NODE *hnode;

    if (!sessionCache)
        return 0;

    /* Remove all sessions from the hash table. */
    hnode = sfxhash_mru_node(sessionCache->hashTable);
    while (hnode)
    {
        idx = (Stream5LWSession *)hnode->data;
        if (!idx)
        {
            sfxhash_free_node(sessionCache->hashTable, hnode);
        }
        else
        {
            Stream5SetRuntimeConfiguration(idx, idx->protocol);
            idx->session_flags |= SSNFLAG_PRUNED;
            DeleteLWSession(sessionCache, idx, "purge whole cache");
        }
        hnode = sfxhash_mru_node(sessionCache->hashTable);
        retCount++;
    }

    return retCount;
}

int DeleteLWSessionCache(Stream5SessionCache *sessionCache)
{
    int retCount = 0;

    if (!sessionCache)
        return 0;

    retCount = PurgeLWSessionCache(sessionCache);

    sfxhash_delete(sessionCache->hashTable);
    free(sessionCache);

    return retCount;
}

static inline int SessionWasBlocked (Stream5LWSession* ssn)
{
    return (ssn->session_flags & (SSNFLAG_DROP_CLIENT|SSNFLAG_DROP_SERVER)) != 0;
}

int PruneLWSessionCache(Stream5SessionCache *sessionCache,
                   uint32_t thetime,
                   Stream5LWSession *save_me,
                   int memCheck)
{
    Stream5LWSession *idx;
    uint32_t pruned = 0;

    if (thetime != 0)
    {
        /* Pruning, look for sessions that have time'd out */
        char got_one;
        idx = (Stream5LWSession *) sfxhash_lru(sessionCache->hashTable);

        if(idx == NULL)
        {
            return 0;
        }

        do
        {
            got_one = 0;
            if(idx == save_me)
            {
                SFXHASH_NODE *lastNode = sfxhash_lru_node(sessionCache->hashTable);
                sfxhash_gmovetofront(sessionCache->hashTable, lastNode);
                lastNode = sfxhash_lru_node(sessionCache->hashTable);
                if ((lastNode) && (lastNode->data != idx))
                {
                    idx = (Stream5LWSession *)lastNode->data;
                    continue;
                }
                else
                {
                    sessionCache->prunes += pruned;
                    return pruned;
                }
            }

            if((idx->last_data_seen+sessionCache->timeoutAggressive) < thetime)
            {
                Stream5LWSession *savidx = idx;

                if(sfxhash_count(sessionCache->hashTable) > 1)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "pruning stale session\n"););
                    savidx->session_flags |= SSNFLAG_TIMEDOUT;
                    DeleteLWSession(sessionCache, savidx, "stale/timeout");

                    idx = (Stream5LWSession *) sfxhash_lru(sessionCache->hashTable);
                    pruned++;
                    got_one = 1;
                }
                else
                {
                    savidx->session_flags |= SSNFLAG_TIMEDOUT;
                    DeleteLWSession(sessionCache, savidx, "stale/timeout/last ssn");
                    pruned++;
                    sessionCache->prunes += pruned;
                    return pruned;
                }
            }
            else
            {
                sessionCache->prunes += pruned;
                return pruned;
            }

            if (pruned > sessionCache->cleanup_sessions)
            {
                /* Don't bother cleaning more than 'n' at a time */
                break;
            }
        } while ((idx != NULL) && (got_one == 1));

        sessionCache->prunes += pruned;
        return pruned;
    }
    else
    {
        /* Free up 'n' sessions at a time until we get under the
         * memcap or free enough sessions to be able to create
         * new ones.
         */
         unsigned int session_count;
#define s5_sessions_in_table() \
     ((session_count = sfxhash_count(sessionCache->hashTable)) > 1)
#define s5_over_session_limit() \
     (session_count > (sessionCache->max_sessions - sessionCache->cleanup_sessions))
#define s5_havent_pruned_yet() \
     (pruned == 0)
#define s5_over_memcap() \
     (mem_in_use > s5_global_eval_config->memcap)

        while (s5_sessions_in_table() &&
               ((!memCheck && (s5_over_session_limit() || s5_havent_pruned_yet())) ||
               (memCheck && s5_over_memcap() )))
        {
            unsigned int i;
            unsigned int blocks = 0;
            DEBUG_WRAP(
                DebugMessage(DEBUG_STREAM,
                    "S5: Pruning session cache by %d ssns for %s: %d/%d\n",
                    sessionCache->cleanup_sessions,
                    memCheck ? "memcap" : "hash limit",
                    mem_in_use,
                    s5_global_eval_config->memcap););

            idx = (Stream5LWSession *) sfxhash_lru(sessionCache->hashTable);

            for (i=0;i<sessionCache->cleanup_sessions &&
                     (sfxhash_count(sessionCache->hashTable) >= blocks+1); i++)
            {
                if ( (idx != save_me) && (!memCheck || !SessionWasBlocked(idx)) )
                {
                    idx->session_flags |= SSNFLAG_PRUNED;
                    DeleteLWSession(sessionCache, idx, memCheck ? "memcap/check" : "memcap/stale");
                    pruned++;
                    idx = (Stream5LWSession *) sfxhash_lru(sessionCache->hashTable);
                }
                else
                {
                    SFXHASH_NODE* lastNode;

                    if ( SessionWasBlocked(idx) )
                        blocks++;

                    lastNode = sfxhash_lru_node(sessionCache->hashTable);
                    sfxhash_gmovetofront(sessionCache->hashTable, lastNode);
                    lastNode = sfxhash_lru_node(sessionCache->hashTable);
                    if ((lastNode) && (lastNode->data == idx))
                    {
                        /* Okay, this session is the only one left */
                        break;
                    }
                    idx = (Stream5LWSession *) sfxhash_lru(sessionCache->hashTable);
                    i--; /* Didn't clean this one */
                }
            }

            /* Nothing (or the one we're working with) in table, couldn't
             * kill it */
            if (!memCheck && (pruned == 0))
            {
                break;
            }
        }
    }
    if (memCheck && pruned)
    {
        LogMessage("S5: Pruned %d sessions from cache for memcap. %d ssns remain.  memcap: %d/%d\n",
            pruned, sfxhash_count(sessionCache->hashTable),
            mem_in_use,
            s5_global_eval_config->memcap);
        DEBUG_WRAP(
            if (sfxhash_count(sessionCache->hashTable) == 1)
            {
                DebugMessage(DEBUG_STREAM, "S5: Pruned, one session remains\n");
            }
        );
    }
    sessionCache->prunes += pruned;
    return pruned;
}

Stream5LWSession *NewLWSession(Stream5SessionCache *sessionCache, Packet *p,
                               SessionKey *key, void *policy)
{
    Stream5LWSession *retSsn = NULL;
    SFXHASH_NODE *hnode;
    StreamFlowData *flowdata;

    hnode = sfxhash_get_node(sessionCache->hashTable, key);
    if (!hnode)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "HashTable full, clean it\n"););
        if (!PruneLWSessionCache(sessionCache, p->pkth->ts.tv_sec, NULL, 0))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "HashTable full, no timeouts, clean it\n"););
            PruneLWSessionCache(sessionCache, 0, NULL, 0);
        }

        /* Should have some freed nodes now */
        hnode = sfxhash_get_node(sessionCache->hashTable, key);
#ifdef DEBUG_MSGS
        if (!hnode)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Problem, no freed nodes\n"););
        }
#endif
    }
    if (hnode && hnode->data)
    {
        retSsn = hnode->data;

        /* Zero everything out */
        memset(retSsn, 0, sizeof(Stream5LWSession));

        /* Save the session key for future use */
        memcpy(&(retSsn->key), key, sizeof(SessionKey));

        retSsn->protocol = key->protocol;
        retSsn->last_data_seen = p->pkth->ts.tv_sec;
        retSsn->flowdata = mempool_alloc(&s5FlowMempool);
        flowdata = retSsn->flowdata->data;
        boInitStaticBITOP(&(flowdata->boFlowbits), giFlowbitSize,
                          flowdata->flowb);

        retSsn->policy = policy;
        retSsn->config = s5_config;
        retSsn->policy_id = getRuntimePolicy();
        ((Stream5Config *)sfPolicyUserDataGet(retSsn->config, retSsn->policy_id))->ref_count++;
    }

    return retSsn;
}

uint32_t HashFunc(SFHASHFCN *p, unsigned char *d, int n)
{
    uint32_t a,b,c;
#ifdef MPLS
    uint32_t tmp = 0;
#endif

#ifdef SUP_IP6
    a = *(uint32_t*)d;         /* IPv6 lo[0] */
    b = *(uint32_t*)(d+4);     /* IPv6 lo[1] */
    c = *(uint32_t*)(d+8);     /* IPv6 lo[2] */

    mix(a,b,c);

    a += *(uint32_t*)(d+12);   /* IPv6 lo[3] */
    b += *(uint32_t*)(d+16);   /* IPv6 hi[0] */
    c += *(uint32_t*)(d+20);   /* IPv6 hi[1] */

    mix(a,b,c);

    a += *(uint32_t*)(d+24);   /* IPv6 hi[2] */
    b += *(uint32_t*)(d+28);   /* IPv6 hi[3] */
    c += *(uint32_t*)(d+32);   /* port lo & port hi */

    mix(a,b,c);

    a += *(uint32_t*)(d+36);   /* vlan, protocol, & pad */
#ifdef MPLS
    tmp = *(uint32_t *)(d+40);
    if( tmp )
    {
        b += tmp;   /* mpls label */
    }
    mix(a,b,c);
#endif
#else
    a = *(uint32_t*)d;         /* IPv4 lo */
    b = *(uint32_t*)(d+4);     /* IPv4 hi */
    c = *(uint32_t*)(d+8);     /* port lo & port hi */

    mix(a,b,c);

    a += *(uint32_t*)(d+12);   /* vlan, protocol, & pad */
#ifdef MPLS
    tmp = *(uint32_t *)(d+16);
    if( tmp )
    {
        b += tmp;   /* mpls label */
    }
    mix(a,b,c);
#endif
#endif

    final(a,b,c);

    return c;
}

int HashKeyCmp(const void *s1, const void *s2, size_t n)
{
#ifndef SPARCV9 /* ie, everything else, use 64bit comparisons */
    uint64_t *a,*b;

    a = (uint64_t*)s1;
    b = (uint64_t*)s2;
    if(*a - *b) return 1;       /* Compares IPv4 lo/hi */
                                /* SUP_IP6 Compares IPv6 low[0,1] */

    a++;
    b++;
    if(*a - *b) return 1;       /* Compares port lo/hi, vlan, protocol, pad */
                                /* SUP_IP6 Compares IPv6 low[2,3] */

#ifdef SUP_IP6
    a++;
    b++;
    if(*a - *b) return 1;       /* SUP_IP6 Compares IPv6 hi[0,1] */

    a++;
    b++;
    if(*a - *b) return 1;       /* SUP_IP6 Compares IPv6 hi[2,3] */

    a++;
    b++;
    if(*a - *b) return 1;       /* SUP_IP6 Compares port lo/hi, vlan, protocol, pad */
#endif

#ifdef MPLS
    a++;
    b++;
    {
        uint32_t *x, *y;
        x = (uint32_t *)a;
        y = (uint32_t *)b;
        //x++;
        //y++;
        if (*x - *y) return 1;  /* Compares mpls label */
    }
#endif

#else /* SPARCV9 */
    uint32_t *a,*b;

    a = (uint32_t*)s1;
    b = (uint32_t*)s2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* Compares IPv4 lo/hi */
                                /* SUP_IP6 Compares IPv6 low[0,1] */

    a+=2;
    b+=2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* Compares port lo/hi, vlan, protocol, pad */
                                /* SUP_IP6 Compares IPv6 low[2,3] */

#ifdef SUP_IP6
    a+=2;
    b+=2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* SUP_IP6 Compares IPv6 hi[0,1] */

    a+=2;
    b+=2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* SUP_IP6 Compares IPv6 hi[2,3] */

    a+=2;
    b+=2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* SUP_IP6 Compares port lo/hi, vlan, protocol, pad */
#endif

#ifdef MPLS
    a+=2;
    b+=2;
    {
        uint32_t *x, *y;
        x = (uint32_t *)a;
        y = (uint32_t *)b;
        //x++;
        //y++;
        if (*x - *y) return 1;  /* Compares mpls label */
    }
#endif
#endif /* SPARCV9 */

    return 0;
}

Stream5SessionCache *InitLWSessionCache(int max_sessions,
                                        uint32_t session_timeout_min,
                                        uint32_t session_timeout_max,
                                        uint32_t cleanup_sessions,
                                        uint32_t cleanup_percent,
                                        Stream5SessionCleanup cleanup_fcn)
{
    Stream5SessionCache *sessionCache = NULL;
    /* Rule of thumb, size should be 1.4 times max to avoid
     * collisions.
     */

    int hashTableSize = max_sessions;
//    int hashTableSize = sfxhash_calcrows((int) (max_sessions * 1.4));

    /* Memory required for 1 node: LW Session + Session Key +
     * Node + NodePtr.
     */
    //int maxSessionMem = max_sessions * (
    //                        sizeof(Stream5LWSession) +
    //                        sizeof(SFXHASH_NODE) +
    //                        sizeof(SessionKey) +
    //                        sizeof(SFXHASH_NODE *));
    /* Memory required for table entries */
    //int tableMem = (hashTableSize +1) * sizeof(SFXHASH_NODE*);

    sessionCache = SnortAlloc(sizeof(Stream5SessionCache));
    if (sessionCache)
    {
        sessionCache->timeoutAggressive = session_timeout_min;
        sessionCache->timeoutNominal = session_timeout_max;
        sessionCache->max_sessions = max_sessions;
        if (cleanup_percent)
        {
            sessionCache->cleanup_sessions = max_sessions * cleanup_percent/100;
        }
        else
        {
            sessionCache->cleanup_sessions = cleanup_sessions;
        }
        if (sessionCache->cleanup_sessions == 0)
        {
            sessionCache->cleanup_sessions = 1;
        }
        sessionCache->cleanup_fcn = cleanup_fcn;

        /* Okay, now create the table */
        sessionCache->hashTable = sfxhash_new(
            hashTableSize,
            sizeof(SessionKey),
            sizeof(Stream5LWSession),
            //maxSessionMem + tableMem, 0, NULL, NULL, 1);
            0, 0, NULL, NULL, 1);

        sfxhash_set_max_nodes(sessionCache->hashTable, max_sessions);
//#ifdef SUP_IP6
        sfxhash_set_keyops(sessionCache->hashTable, HashFunc, HashKeyCmp);
//#endif
    }

    return sessionCache;
}

void PrintLWSessionCache(Stream5SessionCache *sessionCache)
{
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "%lu sessions active\n",
                            sfxhash_count(sessionCache->hashTable)););
}

int
Stream5SetRuntimeConfiguration(
        Stream5LWSession *lwssn,
        uint8_t protocol
        )
{
    Stream5Config *pPolicyConfig;

    if (lwssn == NULL)
        pPolicyConfig = (Stream5Config *)sfPolicyUserDataGet(s5_config, getRuntimePolicy());
    else
        pPolicyConfig = (Stream5Config *)sfPolicyUserDataGet(lwssn->config, lwssn->policy_id);

    if (pPolicyConfig == NULL)
        return -1;

    if (pPolicyConfig->global_config == NULL)
        return -1;

    switch (protocol)
    {
        case IPPROTO_TCP:
            if (pPolicyConfig->tcp_config == NULL)
                return -1;
            s5_tcp_eval_config = pPolicyConfig->tcp_config;
            break;
        case IPPROTO_UDP:
            if (pPolicyConfig->udp_config == NULL)
                return -1;
            s5_udp_eval_config = pPolicyConfig->udp_config;
            break;
        case IPPROTO_ICMP:
            if (pPolicyConfig->icmp_config == NULL)
                return -1;
            s5_icmp_eval_config = pPolicyConfig->icmp_config;
            break;
        default:
            if (pPolicyConfig->ip_config == NULL)
                return -1;
            s5_ip_eval_config = pPolicyConfig->ip_config;
    }

    s5_global_eval_config = pPolicyConfig->global_config;

    return 0;
}

static void checkCacheFlowTimeout(uint32_t flowCount, time_t cur_time, Stream5SessionCache *cache)
{
    uint32_t flowRetiredCount = 0;
    Stream5LWSession *idx;
    SFXHASH_NODE *hnode;

    if (cache)
    {
        while ((hnode = sfxhash_lru_node(cache->hashTable)))
        {
            idx = (Stream5LWSession *)hnode->data;
            if((time_t)(idx->last_data_seen + cache->timeoutNominal) > cur_time)
            {
                break;
            }

            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "retiring stale session\n"););
            idx->session_flags |= SSNFLAG_TIMEDOUT;
            DeleteLWSession(cache, idx, "stale/timeout");

            if (flowRetiredCount++ >= flowCount)
            {
                break;
            }
        }
    }
}

extern Stream5SessionCache *tcp_lws_cache, *udp_lws_cache;

/*get next flow from session cache. */
void checkLWSessionTimeout(uint32_t flowCount, time_t cur_time)
{
    checkCacheFlowTimeout(flowCount, cur_time, tcp_lws_cache);
    checkCacheFlowTimeout(flowCount, cur_time, udp_lws_cache);
    //icmp_lws_cache does not need cleaning

}


