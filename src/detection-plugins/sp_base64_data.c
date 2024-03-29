/*
 ** Copyright (C) 1998-2012 Sourcefire, Inc.
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

/* sp_base64_data
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <errno.h>

#include "sf_types.h"
#include "snort_bounds.h"
#include "rules.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "mstring.h"

#include "snort.h"
#include "profiler.h"
#include "sp_base64_data.h"
#ifdef PERF_PROFILING
PreprocStats base64DataPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "detection_options.h"
#include "detection_util.h"

extern char *file_name;  /* this is the file name from rules.c, generally used
                            for error messages */

extern int file_line;    /* this is the file line number from rules.c that is
                            used to indicate file lines for error messages */

static void Base64DataInit(char *, OptTreeNode *, int);
void Base64DataParse(char *, OptTreeNode *);
int  Base64DataEval(void *option_data, Packet *p);

/****************************************************************************
 *
 * Function: SetupBase64Data()
 *
 * Purpose: Load 'er up
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupBase64Data(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("base64_data", Base64DataInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("base64_data", &base64DataPerfStats, 3, &ruleOTNEvalPerfStats);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: base64_data Setup\n"););
}


/****************************************************************************
 *
 * Function: Base64DataInit(char *, OptTreeNode *, int protocol)
 *
 * Purpose: Generic rule configuration function.  Handles parsing the rule
 *          information and attaching the associated detection function to
 *          the OTN.
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *            protocol => protocol the rule is on (we don't care in this case)
 *
 * Returns: void function
 *
 ****************************************************************************/
static void Base64DataInit(char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    if(otn->ds_list[PLUGIN_BASE64_DECODE] == NULL )
    {
        /*use base64_decode before base64_data*/
        FatalError("%s(%d): base64_decode needs to be specified before base64_data in a rule\n",
                file_name, file_line);
    }

    Base64DataParse(data, otn);

    fpl = AddOptFuncToList(Base64DataEval, otn);
    fpl->type = RULE_OPTION_TYPE_BASE64_DATA;

}



/****************************************************************************
 *
 * Function: Base64DataParse(char *, OptTreeNode *)
 *
 * Purpose: This is the function that is used to process the option keyword's
 *          arguments and attach them to the rule's data structures.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void Base64DataParse(char *data, OptTreeNode *otn)
{
    if (!IsEmptyStr(data))
    {
        FatalError("%s(%d): base64_data takes no arguments\n",
                file_name, file_line);
    }

}


/****************************************************************************
 *
 * Function: Base64DataEval(char *, OptTreeNode *, OptFpList *)
 *
 * Purpose: Use this function to perform the particular detection routine
 *          that this rule keyword is supposed to encompass.
 *
 * Arguments: p => pointer to the decoded packet
 *            otn => pointer to the current rule's OTN
 *            fp_list => pointer to the function pointer list
 *
 * Returns: If the detection test fails, this function *must* return a zero!
 *          On success, it calls the next function in the detection list
 *
 ****************************************************************************/
int Base64DataEval(void *option_data, Packet *p)
{
    int rval = DETECTION_OPTION_NO_MATCH;
    PROFILE_VARS;

    PREPROC_PROFILE_START(base64DataPerfStats);

    if ((p->dsize == 0) || !base64_decode_size )
    {
        PREPROC_PROFILE_END(base64DataPerfStats);
        return rval;
    }

    SetDoePtr(base64_decode_buf, DOE_BUF_STD);
    SetAltDetect(base64_decode_buf, (uint16_t)base64_decode_size);
    rval = DETECTION_OPTION_MATCH;

    PREPROC_PROFILE_END(base64DataPerfStats);
    return rval;
}
