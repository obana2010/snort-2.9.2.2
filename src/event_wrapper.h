/****************************************************************************
 *
 * Copyright (C) 2003-2012 Sourcefire, Inc.
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
 
#ifndef _EVENT_WRAPPER_H
#define _EVENT_WRAPPER_H

#include "log.h"
#include "detect.h"
#include "decode.h"
#include "rules.h"
#include "treenodes.h"

/* 
 * this has been upgarded to reroute traffic to fpLogEvent() 
 * to add support for thresholding, and other rule behaviors 
 * like drop,alert.  This has been updated to allow decoder events
 * which call it to be filtered through fpLogEvent.  This of course
 * requires a rule be writen for each decoder event, and preprocssor event,
 * although preprocessors don't seem to use this much.
 */
uint32_t GenerateSnortEvent(Packet *p,
                            uint32_t gen_id,
                            uint32_t sig_id,
                            uint32_t sig_rev,
                            uint32_t classification,
                            uint32_t priority,
                            char *msg);

OptTreeNode * GenerateSnortEventOtn(
                            uint32_t gen_id,
                            uint32_t sig_id,
                            uint32_t sig_rev,
                            uint32_t classification,
                            uint32_t priority,
                            char *msg );

RuleTreeNode* GenerateSnortEventRtn(OptTreeNode*, tSfPolicyId);

int LogTagData(Packet *p,
               uint32_t gen_id,
               uint32_t sig_id,
               uint32_t sig_rev,
               uint32_t classification,
               uint32_t priority,
               uint32_t event_ref,
               time_t ref_sec,
               char *msg);

#endif /* _EVENT_WRAPPER_H */
