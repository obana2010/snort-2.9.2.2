/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2009-2012 Sourcefire, Inc.
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
 
#ifndef _RATE_FILTER_H_
#define _RATE_FILTER_H_

/* @file  rate_filter.h
 * @brief rate filter interface for Snort
 * @ingroup rate_filter 
 * @author Dilbagh Chahal
*/

/* @ingroup rate_filter 
 * @{
 */
#include "decode.h"
#include "rules.h"
#include "treenodes.h"
#include "rate_filter.h"

RateFilterConfig * RateFilter_ConfigNew(void);
void RateFilter_ConfigFree(RateFilterConfig *);
void RateFilter_Cleanup(void);

int RateFilter_Create(RateFilterConfig *, tSFRFConfigNode *);
void RateFilter_PrintConfig(RateFilterConfig *);

int  RateFilter_Test(OptTreeNode*, Packet*);
void RateFilter_ResetActive(void);

/*@}*/
#endif

