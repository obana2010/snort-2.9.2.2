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
 * Copyright (C) 2005-2012 Sourcefire, Inc.
 *
 * Author: Steven Sturges
 *
 */

/* $Id$ */

#ifndef __SP_DYNAMIC_H_
#define __SP_DYNAMIC_H_

#include "sf_dynamic_define.h"
#include "sf_dynamic_engine.h"
#include "snort.h"
#include "sf_types.h"
#include "sf_engine/sf_snort_plugin_api.h"
#include "detection-plugins/sp_pattern_match.h"

extern char *snort_conf_dir;

typedef struct _DynamicData
{
    void *contextData;
    OTNCheckFunction checkFunction;
    OTNHasFunction hasOptionFunction;
    int contentFlags;
    GetDynamicContentsFunction getDynamicContents;
    GetDynamicPreprocOptFpContentsFunc getPreprocFpContents;
    PatternMatchData *pmds;

} DynamicData;

void SetupDynamic(void);

int RegisterDynamicRule(
    uint32_t gid,
    uint32_t sid,
    void *info,
    OTNCheckFunction,
    OTNHasFunction,
    int contentFlags,
    GetDynamicContentsFunction,
    RuleFreeFunc freeFunc,
    GetDynamicPreprocOptFpContentsFunc
    );

typedef struct _DynamicRuleNode
{
    Rule *rule;
    OTNCheckFunction chkFunc;
    OTNHasFunction hasFunc;
    int contentFlags;
    GetDynamicContentsFunction contentsFunc;
    int converted;
    RuleFreeFunc freeFunc;
    GetDynamicPreprocOptFpContentsFunc preprocFpContentsFunc;
    struct _DynamicRuleNode *next;

} DynamicRuleNode;

void DynamicRuleListFree(DynamicRuleNode *);

#ifdef SNORT_RELOAD
int ReloadDynamicRules(SnortConfig *);
#endif

int DynamicPreprocRuleOptInit(void *);
uint32_t DynamicFlowbitRegister(char *name, int op);
void DynamicFlowbitUnregister(char *name, int op);
int DynamicFlowbitCheck(void *pkt, int op, uint32_t id);
int DynamicAsn1Detect(void *pkt, void *ctxt, const uint8_t *cursor);
int DynamicsfUnfold(const uint8_t *, uint32_t , uint8_t *, uint32_t , uint32_t *);
int Dynamicsfbase64decode(uint8_t *, uint32_t , uint8_t *, uint32_t , uint32_t *);
int DynamicGetAltDetect(uint8_t **, uint16_t *);
void DynamicSetAltDetect(uint8_t *, uint16_t );
int DynamicIsDetectFlag(SFDetectFlagType);
void DynamicDetectFlagDisable(SFDetectFlagType);

int DynamicHasFlow(OptTreeNode *otn);
int DynamicHasFlowbit(OptTreeNode *otn);
int DynamicHasContent(OptTreeNode *otn);
int DynamicHasByteTest(OptTreeNode *otn);
int DynamicHasPCRE(OptTreeNode *otn);

uint32_t DynamicRuleHash(void *d);
int DynamicRuleCompare(void *l, void *r);

#endif  /* __SP_DYNAMIC_H_ */

