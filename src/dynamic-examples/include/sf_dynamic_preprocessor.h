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
 * Dynamic Library Loading for Snort
 *
 */
#ifndef _SF_DYNAMIC_PREPROCESSOR_H_
#define _SF_DYNAMIC_PREPROCESSOR_H_

#include <ctype.h>
#ifdef SF_WCHAR
#include <wchar.h>
#endif
#include "sf_dynamic_meta.h"
#include "ipv6_port.h"
#include "obfuscation.h"

/* specifies that a function does not return
 * used for quieting Visual Studio warnings
 */
#ifdef WIN32
#if _MSC_VER >= 1400
#define NORETURN __declspec(noreturn)
#else
#define NORETURN
#endif
#else
#define NORETURN
#endif

#ifdef PERF_PROFILING
#ifndef PROFILE_PREPROCS_NOREDEF /* Don't redefine this from the main area */
#ifdef PROFILING_PREPROCS
#undef PROFILING_PREPROCS
#endif
#define PROFILING_PREPROCS _dpd.profilingPreprocsFunc()
#endif
#endif

#define PREPROCESSOR_DATA_VERSION 5

#include "sf_dynamic_common.h"
#include "sf_dynamic_engine.h"
#include "stream_api.h"
#include "str_search.h"
#include "obfuscation.h"
#include "attribute_table_api.h"
#include "sfcontrol.h"
#include "idle_processing.h"

#define MINIMUM_DYNAMIC_PREPROC_ID 10000
typedef void (*PreprocessorInitFunc)(char *);
typedef void * (*AddPreprocFunc)(void (*pp_func)(void *, void *), uint16_t, uint32_t, uint32_t);
typedef void * (*AddMetaEvalFunc)(void (*meta_eval_func)(int, const uint8_t *), uint16_t priority, uint32_t preproc_id);
typedef void (*AddPreprocExit)(void (*pp_exit_func) (int, void *), void *arg, uint16_t, uint32_t);
typedef void (*AddPreprocUnused)(void (*pp_unused_func) (int, void *), void *arg, uint16_t, uint32_t);
typedef void (*AddPreprocConfCheck)(void (*pp_conf_chk_func) (void));
typedef int (*AlertQueueAdd)(unsigned int, unsigned int, unsigned int,
                             unsigned int, unsigned int, char *, void *);
typedef uint32_t (*GenSnortEvent)(SFSnortPacket *p, uint32_t gid, uint32_t sid, uint32_t rev,
                                  uint32_t classification, uint32_t priority, char *msg);
#ifdef SNORT_RELOAD
typedef void (*PreprocessorReloadFunc)(char *);
typedef int (*PreprocessorReloadVerifyFunc)(void);
typedef void * (*PreprocessorReloadSwapFunc)(void);
typedef void (*PreprocessorReloadSwapFreeFunc)(void *);
#endif

#ifndef SNORT_RELOAD
typedef void (*PreprocRegisterFunc)(const char *, PreprocessorInitFunc);
#else
typedef void (*PreprocRegisterFunc)(const char *, PreprocessorInitFunc,
                                    PreprocessorReloadFunc,
                                    PreprocessorReloadSwapFunc,
                                    PreprocessorReloadSwapFreeFunc);

typedef void (*AddPreprocReloadVerifyFunc)(PreprocessorReloadVerifyFunc);
#endif
typedef int (*ThresholdCheckFunc)(unsigned int, unsigned int, snort_ip_p, snort_ip_p, long);
typedef void (*InlineDropFunc)(void *);
typedef void (*ActiveEnableFunc)(int);
typedef void (*DisableDetectFunc)(void *);
typedef int (*SetPreprocBitFunc)(void *, uint32_t);
typedef int (*DetectFunc)(void *);
typedef void *(*GetRuleInfoByNameFunc)(char *);
typedef void *(*GetRuleInfoByIdFunc)(int);
typedef int (*printfappendfunc)(char *, int, const char *, ...);
typedef char ** (*TokenSplitFunc)(const char *, const char *, const int, int *, const char);
typedef void (*TokenFreeFunc)(char ***, int);
typedef void (*AddPreprocProfileFunc)(const char *, void *, int, void *);
typedef int (*ProfilingFunc)(void);
typedef int (*PreprocessFunc)(void *);
typedef void (*PreprocStatsRegisterFunc)(const char *, void (*pp_stats_func)(int));
typedef void (*AddPreprocReset)(void (*pp_rst_func) (int, void *), void *arg, uint16_t, uint32_t);
typedef void (*AddPreprocResetStats)(void (*pp_rst_stats_func) (int, void *), void *arg, uint16_t, uint32_t);
typedef void (*AddPreprocReassemblyPktFunc)(void * (*pp_reass_pkt_func)(void), uint32_t);
typedef int (*SetPreprocReassemblyPktBitFunc)(void *, uint32_t);
typedef void (*DisablePreprocessorsFunc)(void *);
#ifdef TARGET_BASED
typedef int16_t (*FindProtocolReferenceFunc)(const char *);
typedef int16_t (*AddProtocolReferenceFunc)(const char *);
typedef int (*IsAdaptiveConfiguredFunc)(tSfPolicyId, int);
#endif
#ifdef SUP_IP6
typedef void (*IP6BuildFunc)(void *, const void *, int);
#define SET_CALLBACK_IP 0
#define SET_CALLBACK_ICMP_ORIG 1
typedef void (*IP6SetCallbacksFunc)(void *, int, char);
#endif
typedef void (*AddKeywordOverrideFunc)(char *, char *, PreprocOptionInit,
        PreprocOptionEval, PreprocOptionCleanup, PreprocOptionHash,
        PreprocOptionKeyCompare, PreprocOptionOtnHandler,
        PreprocOptionFastPatternFunc);
typedef void (*AddKeywordByteOrderFunc)(char *, PreprocOptionByteOrderFunc);

typedef int (*IsPreprocEnabledFunc)(uint32_t);

typedef char * (*PortArrayFunc)(char *, void *, int *);

typedef int (*AlertQueueLog)(void *);
typedef void (*AlertQueueControl)(void);  // reset, push, and pop
typedef tSfPolicyId (*GetPolicyFunc)(void);
typedef void (*SetPolicyFunc)(tSfPolicyId);
typedef tSfPolicyId (*GetPolicyFromIdFunc)(uint16_t );
typedef void (*ChangePolicyFunc)(tSfPolicyId, void *p);
typedef void (*SetFileDataPtrFunc)(uint8_t *,uint16_t );
typedef void (*DetectResetFunc)(uint8_t *,uint16_t );
typedef void (*SetAltDecodeFunc)(uint16_t );
typedef void (*DetectFlagEnableFunc)(SFDetectFlagType);
typedef long (*DynamicStrtol)(const char *, char **, int);
typedef unsigned long(*DynamicStrtoul)(const char *, char **, int);
typedef const char* (*DynamicStrnStr)(const char *, int, const char *);
typedef const char* (*DynamicStrcasestr)(const char *, int, const char *);
typedef int (*DynamicStrncpy)(char *, const char *, size_t );
typedef const char* (*DynamicStrnPbrk)(const char *, int , const char *);

typedef int (*EvalRTNFunc)(void *rtn, void *p, int check_ports);

typedef void* (*EncodeNew)(void);
typedef void (*EncodeDelete)(void*);
typedef void (*EncodeUpdate)(void*);
typedef int (*EncodeFormat)(uint32_t, const void*, void*, int);
typedef bool (*PafEnabledFunc)(void);

typedef char* (*GetLogDirectory)(void);
typedef uint32_t (*GetSnortInstance)(void);

typedef int (*ControlSocketRegisterHandlerFunc)(uint16_t, OOBPreControlFunc, IBControlFunc,
                                                OOBPostControlFunc);

typedef int (*RegisterIdleHandler)(IdleProcessingHandler);
typedef void (*DynamicSendBlockResponse)(void *packet, const uint8_t* buffer, uint32_t buffer_len);
typedef int (*DynamicSetFlowId)(const void* p, uint32_t id);

typedef int (*DynamicIsStrEmpty)(const char * );
typedef void (*AddPeriodicCheck)(void (*pp_check_func) (int, void *), void *arg, uint16_t, uint32_t, uint32_t);
typedef void (*AddPostConfigFuncs)(void (*pp_post_config_func) (void *), void *arg);

#define ENC_DYN_FWD 0x80000000
#define ENC_DYN_NET 0x10000000

/* Info Data passed to dynamic preprocessor plugin must include:
 * version
 * Pointer to AltDecodeBuffer
 * Pointer to HTTP URI Buffers
 * Pointer to functions to log Messages, Errors, Fatal Errors
 * Pointer to function to add preprocessor to list of configure Preprocs
 * Pointer to function to regsiter preprocessor configuration keyword
 * Pointer to function to create preprocessor alert
 */
typedef struct _DynamicPreprocessorData
{
    int version;
    int size;

    SFDataBuffer *altBuffer;
    SFDataPointer *altDetect;
    SFDataPointer *fileDataBuf;
    UriInfo *uriBuffers[HTTP_BUFFER_MAX];

    LogMsgFunc logMsg;
    LogMsgFunc errMsg;
    LogMsgFunc fatalMsg;
    DebugMsgFunc debugMsg;

    PreprocRegisterFunc registerPreproc;
    AddPreprocFunc addPreproc;
    GetSnortInstance getSnortInstance;
    AddPreprocExit addPreprocExit;
    AddPreprocConfCheck addPreprocConfCheck;
    RegisterPreprocRuleOpt preprocOptRegister;
    AddPreprocProfileFunc addPreprocProfileFunc;
    ProfilingFunc profilingPreprocsFunc;
    void *totalPerfStats;

    AlertQueueAdd alertAdd;
    GenSnortEvent genSnortEvent;
    ThresholdCheckFunc thresholdCheck;
    InlineDropFunc  inlineDropAndReset;
    ActiveEnableFunc activeSetEnabled;

    DetectFunc detect;
    DisableDetectFunc disableDetect;
    DisableDetectFunc disableAllDetect;

    SetPreprocBitFunc setPreprocBit;

    StreamAPI *streamAPI;
    SearchAPI *searchAPI;

    char **config_file;
    int *config_line;
    printfappendfunc printfappend;
    TokenSplitFunc tokenSplit;
    TokenFreeFunc tokenFree;

    GetRuleInfoByNameFunc getRuleInfoByName;
    GetRuleInfoByIdFunc getRuleInfoById;
#ifdef SF_WCHAR
    DebugWideMsgFunc debugWideMsg;
#endif

    PreprocessFunc preprocess;

    char **debugMsgFile;
    int *debugMsgLine;

    PreprocStatsRegisterFunc registerPreprocStats;
    AddPreprocReset addPreprocReset;
    AddPreprocResetStats addPreprocResetStats;
    DisablePreprocessorsFunc disablePreprocessors;

#ifdef SUP_IP6
    IP6BuildFunc ip6Build;
    IP6SetCallbacksFunc ip6SetCallbacks;
#endif

    AlertQueueLog logAlerts;
    AlertQueueControl resetAlerts;
    AlertQueueControl pushAlerts;
    AlertQueueControl popAlerts;

#ifdef TARGET_BASED
    FindProtocolReferenceFunc findProtocolReference;
    AddProtocolReferenceFunc addProtocolReference;
    IsAdaptiveConfiguredFunc isAdaptiveConfigured;
#endif

    AddKeywordOverrideFunc preprocOptOverrideKeyword;
    AddKeywordByteOrderFunc preprocOptByteOrderKeyword;
    IsPreprocEnabledFunc isPreprocEnabled;

#ifdef SNORT_RELOAD
    AddPreprocReloadVerifyFunc addPreprocReloadVerify;
#endif

    PortArrayFunc portObjectCharPortArray;

    GetPolicyFunc getRuntimePolicy;
    GetPolicyFunc getParserPolicy;
    GetPolicyFunc getDefaultPolicy;
    SetPolicyFunc setParserPolicy;
    SetFileDataPtrFunc setFileDataPtr;
    DetectResetFunc DetectReset;
    SetAltDecodeFunc SetAltDecode;
    GetAltDetectFunc GetAltDetect;
    SetAltDetectFunc SetAltDetect;
    IsDetectFlagFunc Is_DetectFlag;
    DetectFlagDisableFunc DetectFlag_Disable;
    DynamicStrtol SnortStrtol;
    DynamicStrtoul SnortStrtoul;
    DynamicStrnStr SnortStrnStr;
    DynamicStrncpy SnortStrncpy;
    DynamicStrnPbrk SnortStrnPbrk;
    DynamicStrcasestr SnortStrcasestr;
    EvalRTNFunc fpEvalRTN;

    ObfuscationApi *obApi;

    EncodeNew encodeNew;
    EncodeDelete encodeDelete;
    EncodeFormat encodeFormat;
    EncodeUpdate encodeUpdate;

    AddPreprocFunc addDetect;
    PafEnabledFunc isPafEnabled;

#ifdef TARGET_BASED
    HostAttributeTableApi *hostAttributeTableApi;
#endif

    GetLogDirectory getLogDirectory;

    ControlSocketRegisterHandlerFunc controlSocketRegisterHandler;
    RegisterIdleHandler registerIdleHandler;

    GetPolicyFromIdFunc getPolicyFromId;
    ChangePolicyFunc changeRuntimePolicy;
    InlineDropFunc  inlineForceDropPacket;
    InlineDropFunc  inlineForceDropAndReset;
    DynamicIsStrEmpty SnortIsStrEmpty;
    AddMetaEvalFunc addMetaEval;
    DynamicSendBlockResponse dynamicSendBlockResponse;
    DynamicSetFlowId dynamicSetFlowId;
    AddPeriodicCheck addPeriodicCheck;
    AddPostConfigFuncs addPostConfigFunc;
    char **snort_conf_dir;

} DynamicPreprocessorData;

/* Function prototypes for Dynamic Preprocessor Plugins */
void CloseDynamicPreprocessorLibs(void);
int LoadDynamicPreprocessor(char *library_name, int indent);
void LoadAllDynamicPreprocessors(char *path);
typedef int (*InitPreprocessorLibFunc)(DynamicPreprocessorData *);

int InitDynamicPreprocessors(void);
void RemoveDuplicatePreprocessorPlugins(void);

/* This was necessary because of static code analysis not recognizing that
 * fatalMsg did not return - use instead of fatalMsg
 */
NORETURN void DynamicPreprocessorFatalMessage(const char *format, ...);

extern DynamicPreprocessorData _dpd;

#endif /* _SF_DYNAMIC_PREPROCESSOR_H_ */
