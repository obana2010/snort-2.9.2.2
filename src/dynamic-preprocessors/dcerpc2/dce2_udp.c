/****************************************************************************
 * Copyright (C) 2008-2012 Sourcefire, Inc.
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
 ****************************************************************************
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "dce2_udp.h"
#include "snort_dce2.h"
#include "dce2_cl.h"
#include "dce2_memory.h"
#include "dce2_stats.h"
#include "sf_types.h"

/********************************************************************
 * Extern variables
 ********************************************************************/
extern DCE2_Stats dce2_stats;

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
DCE2_UdpSsnData * DCE2_UdpSsnInit(void)
{
    DCE2_UdpSsnData *usd = DCE2_Alloc(sizeof(DCE2_UdpSsnData), DCE2_MEM_TYPE__UDP_SSN);

    if (usd == NULL)
        return NULL;

    DCE2_ResetRopts(&usd->sd.ropts);

    dce2_stats.udp_sessions++;

    return usd;
}

/********************************************************************
 * Function: DCE2_UdpProcess()
 *
 * Purpose: Main entry point for DCE/RPC over UDP processing.
 *
 * Arguments:
 *  DCE2_UdpSsnData * - a pointer to the data structure associated
 *                      with this session.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_UdpProcess(DCE2_UdpSsnData *usd)
{
    dce2_stats.udp_pkts++;
    DCE2_ClProcess(&usd->sd, &usd->cl_tracker);
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_UdpDataFree(DCE2_UdpSsnData *usd)
{
    if (usd == NULL)
        return;

    DCE2_ClCleanTracker(&usd->cl_tracker);
}

/********************************************************************
 * Function: DCE2_UdpSsnFree()
 *
 * Purpose: Callback to session for freeing sessiond data.
 *
 * Arguments:
 *  void * - pointer to the memory to be freed.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_UdpSsnFree(void *data)
{
    DCE2_UdpSsnData *usd = (DCE2_UdpSsnData *)data;

    if (usd == NULL)
        return;

    DCE2_UdpDataFree(usd);
    DCE2_Free((void *)usd, sizeof(DCE2_UdpSsnData), DCE2_MEM_TYPE__UDP_SSN);
}

