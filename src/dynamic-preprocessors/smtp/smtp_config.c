/****************************************************************************
 *
 * Copyright (C) 2005-2012 Sourcefire, Inc.
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

/***************************************************************************
 * smtp_config.c
 *
 * Author: Andy Mullican
 * Author: Todd Wease
 *
 * Description:
 *
 * Handle configuration of the SMTP preprocessor
 *
 * Entry point functions:
 *
 *    SMTP_ParseArgs()
 *
 ***************************************************************************/

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "snort_smtp.h"
#include "smtp_config.h"
#include "snort_bounds.h"
#include "sf_dynamic_preprocessor.h"
#include "sfPolicy.h"


/*  Global variable to hold configuration */
extern SMTPConfig **smtp_config;

extern const SMTPToken smtp_known_cmds[];

/* Private functions */
static int  ProcessPorts(SMTPConfig *, char *, int);
static int ProcessCmds(SMTPConfig *, char *, int, int);
static int  GetCmdId(SMTPConfig *, char *);
static int  AddCmd(SMTPConfig *, char *name);
static int  ProcessAltMaxCmdLen(SMTPConfig *, char *, int);
static int  ProcessMaxMimeMem(SMTPConfig *, char *, int);
static int  ProcessSmtpMemcap(SMTPConfig *, char *, int);
static int  ProcessMaxMimeDepth(SMTPConfig *, char *, int);
static int  ProcessLogDepth(SMTPConfig *, char *, int);
static int  ProcessXlink2State(SMTPConfig *, char *, int);
static int ProcessDecodeDepth(SMTPConfig *, char *, int , char *, DecodeType );

/*
 * Function: SMTP_ParseArgs(char *)
 *
 * Purpose: Process the preprocessor arguments from the rules file and
 *          initialize the preprocessor's data struct.  This function doesn't
 *          have to exist if it makes sense to parse the args in the init
 *          function.
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 *
 */
void SMTP_ParseArgs(SMTPConfig *config, char *args)
{
    int ret = 0;
    char *arg;
    char *value;
    char errStr[ERRSTRLEN];
    int errStrLen = ERRSTRLEN;
    int b64_option = 0;
    int deprecated_options = 0;

    if ((config == NULL) || (args == NULL))
        return;

    config->ports[SMTP_DEFAULT_SERVER_PORT / 8]     |= 1 << (SMTP_DEFAULT_SERVER_PORT % 8);
    config->ports[XLINK2STATE_DEFAULT_PORT / 8]     |= 1 << (XLINK2STATE_DEFAULT_PORT % 8);
    config->ports[SMTP_DEFAULT_SUBMISSION_PORT / 8] |= 1 << (SMTP_DEFAULT_SUBMISSION_PORT % 8);
    config->inspection_type = SMTP_STATELESS;
    config->max_command_line_len = DEFAULT_MAX_COMMAND_LINE_LEN;
    config->max_header_line_len = DEFAULT_MAX_HEADER_LINE_LEN;
    config->max_response_line_len = DEFAULT_MAX_RESPONSE_LINE_LEN;
    config->max_mime_depth = DEFAULT_MAX_MIME_DEPTH;
    config->max_mime_mem = DEFAULT_MAX_MIME_MEM;
    config->memcap = DEFAULT_SMTP_MEMCAP;
    config->alert_xlink2state = 1;
    config->print_cmds = 1;
    config->enable_mime_decoding = 0;
    config->b64_depth = DEFAULT_MAX_MIME_DEPTH;
    config->qp_depth = DEFAULT_MAX_MIME_DEPTH;
    config->uu_depth = DEFAULT_MAX_MIME_DEPTH;
    config->bitenc_depth = DEFAULT_MAX_MIME_DEPTH;
    config->max_depth = MIN_DEPTH;
    config->log_filename = 0;
    config->log_mailfrom = 0;
    config->log_rcptto = 0;
    config->log_email_hdrs = 0;
    config->email_hdrs_log_depth = DEFAULT_LOG_DEPTH;

    config->cmd_config = (SMTPCmdConfig *)calloc(CMD_LAST, sizeof(SMTPCmdConfig));
    if (config->cmd_config == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory for SMTP "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    *errStr = '\0';

    arg = strtok(args, CONF_SEPARATORS);

    while ( arg != NULL )
    {
        if ( !strcasecmp(CONF_PORTS, arg) )
        {
            ret = ProcessPorts(config, errStr, errStrLen);
        }
        else if ( !strcasecmp(CONF_INSPECTION_TYPE, arg) )
        {
            value = strtok(NULL, CONF_SEPARATORS);
            if ( value == NULL )
            {
                return;
            }
            if ( !strcasecmp(CONF_STATEFUL, value) )
            {
                config->inspection_type = SMTP_STATEFUL;
            }
            else
            {
                config->inspection_type = SMTP_STATELESS;
            }
        }
        else if ( !strcasecmp(CONF_NORMALIZE, arg) )
        {
            value = strtok(NULL, CONF_SEPARATORS);
            if ( value == NULL )
            {
                return;
            }
            if ( !strcasecmp(CONF_NONE, value) )
            {
                config->normalize = NORMALIZE_NONE;
            }
            else if ( !strcasecmp(CONF_ALL, value) )
            {
                config->normalize = NORMALIZE_ALL;
            }
            else
            {
                config->normalize = NORMALIZE_CMDS;
            }
        }
        else if ( !strcasecmp(CONF_IGNORE_DATA, arg) )
        {
            config->ignore_data = 1;
        }
        else if ( !strcasecmp(CONF_IGNORE_TLS_DATA, arg) )
        {
            config->ignore_tls_data = 1;
        }
        else if ( !strcasecmp(CONF_MAX_COMMAND_LINE_LEN, arg) )
        {
            char *endptr;

            value = strtok(NULL, CONF_SEPARATORS);
            if ( value == NULL )
                return;

            config->max_command_line_len = strtol(value, &endptr, 10);
        }
        else if ( !strcasecmp(CONF_MAX_HEADER_LINE_LEN, arg) )
        {
            char *endptr;

            value = strtok(NULL, CONF_SEPARATORS);
            if ( value == NULL )
                return;

            config->max_header_line_len = strtol(value, &endptr, 10);
        }
        else if ( !strcasecmp(CONF_MAX_RESPONSE_LINE_LEN, arg) )
        {
            char *endptr;

            value = strtok(NULL, CONF_SEPARATORS);
            if ( value == NULL )
                return;

            config->max_response_line_len = strtol(value, &endptr, 10);
        }
        else if ( !strcasecmp(CONF_NO_ALERTS, arg) )
        {
            config->no_alerts = 1;
        }
        else if ( !strcasecmp(CONF_ALERT_UNKNOWN_CMDS, arg) )
        {
            config->alert_unknown_cmds = 1;
        }
        else if ( !strcasecmp(CONF_INVALID_CMDS, arg) )
        {
            /* Parse disallowed commands */
            ret = ProcessCmds(config, errStr, errStrLen, ACTION_ALERT);
        }
        else if ( !strcasecmp(CONF_VALID_CMDS, arg) )
        {
            /* Parse allowed commands */
            ret = ProcessCmds(config, errStr, errStrLen, ACTION_NO_ALERT);
        }
        else if ( !strcasecmp(CONF_NORMALIZE_CMDS, arg) )
        {
            /* Parse normalized commands */
            ret = ProcessCmds(config, errStr, errStrLen, ACTION_NORMALIZE);
        }
        else if ( !strcasecmp(CONF_ALT_MAX_COMMAND_LINE_LEN, arg) )
        {
            /* Parse max line len for commands */
            ret = ProcessAltMaxCmdLen(config, errStr, errStrLen);
        }
        else if ( !strcasecmp(CONF_SMTP_MEMCAP, arg) )
        {
            ret = ProcessSmtpMemcap(config, errStr, errStrLen);
        }
        else if ( !strcasecmp(CONF_MAX_MIME_MEM, arg) )
        {
            ret = ProcessMaxMimeMem(config, errStr, errStrLen);
        }
        else if ( !strcasecmp(CONF_MAX_MIME_DEPTH, arg) )
        {
            deprecated_options = 1;
            _dpd.logMsg("WARNING: %s(%d) => The SMTP config option 'max_mime_depth' is deprecated.\n",
                            *(_dpd.config_file), *(_dpd.config_line));
            if(!b64_option)
                ret = ProcessMaxMimeDepth(config, errStr, errStrLen);
        }
        else if ( !strcasecmp(CONF_ENABLE_MIME_DECODING, arg) )
        {
            deprecated_options = 1;
            _dpd.logMsg("WARNING: %s(%d) => The SMTP config option 'enable_mime_decoding' is deprecated.\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
            if(!b64_option)
                config->enable_mime_decoding = 1;
        }
        else if ( !strcasecmp(CONF_DISABLED, arg) )
        {
            config->disabled = 1;
        }
        else if ( !strcasecmp(CONF_XLINK2STATE, arg) )
        {
            ret = ProcessXlink2State(config, errStr, errStrLen);
        }
        else if ( !strcasecmp(CONF_LOG_FILENAME, arg) )
        {
            config->log_filename = 1;
        }
        else if ( !strcasecmp(CONF_LOG_MAIL_FROM, arg) )
        {
            config->log_mailfrom = 1;
        }
        else if ( !strcasecmp(CONF_LOG_RCPT_TO, arg) )
        {
            config->log_rcptto = 1;
        }
        else if ( !strcasecmp(CONF_LOG_EMAIL_HDRS, arg) )
        {
            config->log_email_hdrs = 1;
        }
        else if ( !strcasecmp(CONF_EMAIL_HDRS_LOG_DEPTH, arg) )
        {
            ret = ProcessLogDepth(config, errStr, errStrLen);
        }

        else if ( !strcasecmp(CONF_PRINT_CMDS, arg) )
        {
            config->print_cmds = 1;
        }
        else if ( !strcasecmp(CONF_B64_DECODE, arg) )
        {
            b64_option = 1;
            ret = ProcessDecodeDepth(config, errStr, errStrLen, CONF_B64_DECODE, DECODE_B64);
        }
        else if ( !strcasecmp(CONF_QP_DECODE, arg) )
        {
            ret = ProcessDecodeDepth(config, errStr, errStrLen, CONF_QP_DECODE, DECODE_QP);
        }
        else if ( !strcasecmp(CONF_UU_DECODE, arg) )
        {
            ret = ProcessDecodeDepth(config, errStr, errStrLen, CONF_UU_DECODE, DECODE_UU);
        }
        else if ( !strcasecmp(CONF_BITENC_DECODE, arg) )
        {
            ret = ProcessDecodeDepth(config, errStr, errStrLen, CONF_BITENC_DECODE, DECODE_BITENC);
        }
        else
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Unknown SMTP configuration option %s\n",
                                            *(_dpd.config_file), *(_dpd.config_line), arg);
        }

        if (ret == -1)
        {
            /*
            **  Fatal Error, log error and exit.
            */
            if (*errStr)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => %s\n",
                                                *(_dpd.config_file), *(_dpd.config_line), errStr);
            }
            else
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Undefined Error.\n",
                                                *(_dpd.config_file), *(_dpd.config_line));
            }
        }

        /*  Get next token */
        arg = strtok(NULL, CONF_SEPARATORS);
    }

    if(!b64_option)
    {
        if(config->enable_mime_decoding)
            config->b64_depth = config->max_mime_depth;
    }
    else if(deprecated_options)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Cannot specify 'enable_mime_decoding' or 'max_mime_depth' with "
                                       "'b64_decode_depth'\n",
                                     *(_dpd.config_file), *(_dpd.config_line), arg);
    }

    if(!config->email_hdrs_log_depth)
    {
        if(config->log_email_hdrs)
        {
            _dpd.logMsg("WARNING: %s(%d) => 'log_email_hdrs' enabled with 'email_hdrs_log_depth' = 0."
                    "Email headers won't be logged. Please set 'email_hdrs_log_depth' > 0 to enable logging.\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }
        config->log_email_hdrs = 0;
    }

}
int SMTP_IsDecodingEnabled(SMTPConfig *pPolicyConfig)
{
    if( (pPolicyConfig->b64_depth > -1) || (pPolicyConfig->qp_depth > -1)
         || (pPolicyConfig->uu_depth > -1) || (pPolicyConfig->bitenc_depth > -1) )
    {
        return 0;
    }
    else
        return -1;

}


void SMTP_CheckConfig(SMTPConfig *pPolicyConfig, tSfPolicyUserContextId context)
{
    int max = -1;
    SMTPConfig *defaultConfig =
                (SMTPConfig *)sfPolicyUserDataGetDefault(context);

    if (pPolicyConfig == defaultConfig)
    {
        if (!pPolicyConfig->max_mime_mem)
            pPolicyConfig->max_mime_mem = DEFAULT_MAX_MIME_MEM;

        if(!pPolicyConfig->b64_depth || !pPolicyConfig->qp_depth
                || !pPolicyConfig->uu_depth || !pPolicyConfig->bitenc_depth)
        {
            pPolicyConfig->max_depth = MAX_DEPTH;
            return;
        }
        else
        {
            if(max < pPolicyConfig->b64_depth)
                max = pPolicyConfig->b64_depth;

            if(max < pPolicyConfig->qp_depth)
                max = pPolicyConfig->qp_depth;

            if(max < pPolicyConfig->bitenc_depth)
                max = pPolicyConfig->bitenc_depth;

            if(max < pPolicyConfig->uu_depth)
                max = pPolicyConfig->uu_depth;

            pPolicyConfig->max_depth = max;
        }


        if (!pPolicyConfig->memcap)
            pPolicyConfig->memcap = DEFAULT_SMTP_MEMCAP;

        if(pPolicyConfig->disabled && !pPolicyConfig->email_hdrs_log_depth)
            pPolicyConfig->email_hdrs_log_depth = DEFAULT_LOG_DEPTH;

    }
    else if (defaultConfig == NULL)
    {
        if (pPolicyConfig->max_mime_mem)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: max_mime_mem must be "
                   "configured in the default config.\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }

        if (pPolicyConfig->b64_depth > -1)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: b64_decode_depth must be "
                   "configured in the default config.\n",
                  *(_dpd.config_file), *(_dpd.config_line));
        }

        if (pPolicyConfig->qp_depth > -1)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: qp_decode_depth must be "
                       "configured in the default config.\n",
                      *(_dpd.config_file), *(_dpd.config_line));
        }

        if (pPolicyConfig->uu_depth > -1)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: uu_decode_depth must be "
                       "configured in the default config.\n",
                      *(_dpd.config_file), *(_dpd.config_line));
        }

        if (pPolicyConfig->bitenc_depth > -1)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: bitenc_decode_depth must be "
                       "configured in the default config.\n",
                      *(_dpd.config_file), *(_dpd.config_line));
        }

        if (pPolicyConfig->memcap)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: memcap must be "
                    "configured in the default config.\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }

        if(pPolicyConfig->log_email_hdrs && pPolicyConfig->email_hdrs_log_depth)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: email_hdrs_log_depth must be "
                    "configured in the default config.\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }

    }
    else
    {
        pPolicyConfig->max_mime_mem = defaultConfig->max_mime_mem;
        pPolicyConfig->max_depth = defaultConfig->max_depth;
        pPolicyConfig->memcap = defaultConfig->memcap;
        pPolicyConfig->email_hdrs_log_depth = defaultConfig->email_hdrs_log_depth;
        if(pPolicyConfig->disabled)
        {
            pPolicyConfig->b64_depth = defaultConfig->b64_depth;
            pPolicyConfig->qp_depth = defaultConfig->qp_depth;
            pPolicyConfig->uu_depth = defaultConfig->uu_depth;
            pPolicyConfig->bitenc_depth = defaultConfig->bitenc_depth;
            return;
        }
        if(!pPolicyConfig->b64_depth && defaultConfig->b64_depth)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: Cannot enable unlimited Base64 decoding"
                                    " in non-default config without turning on unlimited Base64 decoding in the default "
                                    " config.\n",
                                    *(_dpd.config_file), *(_dpd.config_line));
        }
        else if(defaultConfig->b64_depth && (pPolicyConfig->b64_depth > defaultConfig->b64_depth))
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: b64_decode_depth value %d in non-default config"
                                       " cannot exceed default config's value %d.\n",
                                       *(_dpd.config_file), *(_dpd.config_line), pPolicyConfig->b64_depth, defaultConfig->b64_depth);
        }

        if(!pPolicyConfig->qp_depth && defaultConfig->qp_depth)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: Cannot enable unlimited Quoted-Printable decoding"
                                        " in non-default config without turning on unlimited Quoted-Printable decoding in the default "
                                        " config.\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
        }
        else if(defaultConfig->qp_depth && (pPolicyConfig->qp_depth > defaultConfig->qp_depth))
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: qp_decode_depth value %d in non-default config"
                                           " cannot exceed default config's value %d.\n",
                                           *(_dpd.config_file), *(_dpd.config_line), pPolicyConfig->qp_depth, defaultConfig->qp_depth);
        }

        if(!pPolicyConfig->uu_depth && defaultConfig->uu_depth )
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: Cannot enable unlimited Unix-to-Unix decoding"
                                " in non-default config without turning on unlimited Unix-to-Unix decoding in the default "
                                " config.\n",
                                *(_dpd.config_file), *(_dpd.config_line));
        }
        else if(defaultConfig->uu_depth && (pPolicyConfig->uu_depth > defaultConfig->uu_depth))
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: uu_decode_depth value %d in non-default config"
                                   " cannot exceed default config's value %d.\n",
                                   *(_dpd.config_file), *(_dpd.config_line), pPolicyConfig->uu_depth, defaultConfig->uu_depth);
        }

        if(!pPolicyConfig->bitenc_depth && defaultConfig->bitenc_depth)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: Cannot enable unlimited 7bit/8bit/binary extraction"
                                        " in non-default config without turning on unlimited 7bit/8bit/binary extraction in the default "
                                        " config.\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
        }
        else if(defaultConfig->bitenc_depth && (pPolicyConfig->bitenc_depth > defaultConfig->bitenc_depth))
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: bitenc_decode_depth value %d in non-default config "
                                           " cannot exceed default config's value.\n",
                                           *(_dpd.config_file), *(_dpd.config_line), pPolicyConfig->bitenc_depth, defaultConfig->bitenc_depth);
        }

    }
}

void SMTP_PrintConfig(SMTPConfig *config)
{
    int i;
    const SMTPToken *cmd;
    char buf[8192];

    if (config == NULL)
        return;

    memset(&buf[0], 0, sizeof(buf));

    _dpd.logMsg("SMTP Config:\n");

    if(config->disabled)
    {
        _dpd.logMsg("    SMTP: INACTIVE\n");
    }

    snprintf(buf, sizeof(buf) - 1, "    Ports: ");

    for (i = 0; i < 65536; i++)
    {
        if (config->ports[i / 8] & (1 << (i % 8)))
        {
            _dpd.printfappend(buf, sizeof(buf) - 1, "%d ", i);
        }
    }

    _dpd.logMsg("%s\n", buf);

    _dpd.logMsg("    Inspection Type: %s\n",
                config->inspection_type ? "Stateful" : "Stateless");

    snprintf(buf, sizeof(buf) - 1, "    Normalize: ");

    switch (config->normalize)
    {
        case NORMALIZE_ALL:
            _dpd.printfappend(buf, sizeof(buf) - 1, "all");
            break;
        case NORMALIZE_NONE:
            _dpd.printfappend(buf, sizeof(buf) - 1, "none");
            break;
        case NORMALIZE_CMDS:
            if (config->print_cmds)
            {
                for (cmd = config->cmds; cmd->name != NULL; cmd++)
                {
                    if (config->cmd_config[cmd->search_id].normalize)
                    {
                        _dpd.printfappend(buf, sizeof(buf) - 1, "%s ", cmd->name);
                    }
                }
            }
            else
            {
                _dpd.printfappend(buf, sizeof(buf) - 1, "cmds");
            }

            break;
    }

    _dpd.logMsg("%s\n", buf);

    _dpd.logMsg("    Ignore Data: %s\n",
                config->ignore_data ? "Yes" : "No");
    _dpd.logMsg("    Ignore TLS Data: %s\n",
                config->ignore_tls_data ? "Yes" : "No");
    _dpd.logMsg("    Ignore SMTP Alerts: %s\n",
                config->no_alerts ? "Yes" : "No");

    if (!config->no_alerts)
    {
        snprintf(buf, sizeof(buf) - 1, "    Max Command Line Length: ");

        if (config->max_command_line_len == 0)
            _dpd.printfappend(buf, sizeof(buf) - 1, "Unlimited");
        else
            _dpd.printfappend(buf, sizeof(buf) - 1, "%d", config->max_command_line_len);

        _dpd.logMsg("%s\n", buf);


        if (config->print_cmds)
        {
            int max_line_len_count = 0;
            int max_line_len = 0;

            snprintf(buf, sizeof(buf) - 1, "    Max Specific Command Line Length: ");

            for (cmd = config->cmds; cmd->name != NULL; cmd++)
            {
                max_line_len = config->cmd_config[cmd->search_id].max_line_len;

                if (max_line_len != 0)
                {
                    if (max_line_len_count % 5 == 0)
                    {
                        _dpd.logMsg("%s\n", buf);
                        snprintf(buf, sizeof(buf) - 1, "       %s:%d ", cmd->name, max_line_len);
                    }
                    else
                    {
                        _dpd.printfappend(buf, sizeof(buf) - 1, "%s:%d ", cmd->name, max_line_len);
                    }

                    max_line_len_count++;
                }
            }

            if (max_line_len_count == 0)
                _dpd.logMsg("%sNone\n", buf);
            else
                _dpd.logMsg("%s\n", buf);
        }

        snprintf(buf, sizeof(buf) - 1, "    Max Header Line Length: ");

        if (config->max_header_line_len == 0)
            _dpd.logMsg("%sUnlimited\n", buf);
        else
            _dpd.logMsg("%s%d\n", buf, config->max_header_line_len);

        snprintf(buf, sizeof(buf) - 1, "    Max Response Line Length: ");

        if (config->max_response_line_len == 0)
            _dpd.logMsg("%sUnlimited\n", buf);
        else
            _dpd.logMsg("%s%d\n", buf, config->max_response_line_len);
    }

    _dpd.logMsg("    X-Link2State Alert: %s\n",
                config->alert_xlink2state ? "Yes" : "No");
    if (config->alert_xlink2state)
    {
        _dpd.logMsg("    Drop on X-Link2State Alert: %s\n",
                    config->drop_xlink2state ? "Yes" : "No");
    }

    if (config->print_cmds && !config->no_alerts)
    {
        int alert_count = 0;

        snprintf(buf, sizeof(buf) - 1, "    Alert on commands: ");

        for (cmd = config->cmds; cmd->name != NULL; cmd++)
        {
            if (config->cmd_config[cmd->search_id].alert)
            {
                _dpd.printfappend(buf, sizeof(buf) - 1, "%s ", cmd->name);
                alert_count++;
            }
        }

        if (alert_count == 0)
        {
            _dpd.logMsg("%sNone\n", buf);
        }
        else
        {
            _dpd.logMsg("%s\n", buf);
        }
    }
    _dpd.logMsg("    Alert on unknown commands: %s\n",
            config->alert_unknown_cmds ? "Yes" : "No");

    _dpd.logMsg("    SMTP Memcap: %u\n",
            config->memcap);

    _dpd.logMsg("    MIME Max Mem: %d\n",
            config->max_mime_mem);

    if(config->b64_depth > -1)
    {
        _dpd.logMsg("    Base64 Decoding: %s\n", "Enabled");
        switch(config->b64_depth)
        {
            case 0:
                _dpd.logMsg("    Base64 Decoding Depth: %s\n", "Unlimited");
                break;
            default:
                _dpd.logMsg("    Base64 Decoding Depth: %d\n", config->b64_depth);
                break;
        }
    }
    else
        _dpd.logMsg("    Base64 Decoding: %s\n", "Disabled");

    if(config->qp_depth > -1)
    {
        _dpd.logMsg("    Quoted-Printable Decoding: %s\n","Enabled");
        switch(config->qp_depth)
        {
            case 0:
                _dpd.logMsg("    Quoted-Printable Decoding Depth: %s\n", "Unlimited");
                break;
            default:
                _dpd.logMsg("    Quoted-Printable Decoding Depth: %d\n", config->qp_depth);
                break;
        }
    }
    else
        _dpd.logMsg("    Quoted-Printable Decoding: %s\n", "Disabled");

    if(config->uu_depth > -1)
    {
        _dpd.logMsg("    Unix-to-Unix Decoding: %s\n","Enabled");
        switch(config->uu_depth)
        {
            case 0:
                _dpd.logMsg("    Unix-to-Unix Decoding Depth: %s\n", "Unlimited");
                break;
            default:
                _dpd.logMsg("    Unix-to-Unix Decoding Depth: %d\n", config->uu_depth);
                break;
        }
    }
    else
        _dpd.logMsg("    Unix-to-Unix Decoding: %s\n", "Disabled");

    if(config->bitenc_depth > -1)
    {
        _dpd.logMsg("    7bit/8bit/binary Extraction: %s\n","Enabled");
        switch(config->bitenc_depth)
        {
            case 0:
                _dpd.logMsg("    7bit/8bit/binary Extraction Depth: %s\n", "Unlimited");
                break;
            default:
                _dpd.logMsg("    7bit/8bit/binary Extraction Depth: %d\n", config->bitenc_depth);
                break;
        }
    }
    else
        _dpd.logMsg("    7bit/8bit/binary Extraction: %s\n", "Disabled");

    _dpd.logMsg("    Log Attachment filename: %s\n",
                config->log_filename ? "Enabled" : "Not Enabled");

    _dpd.logMsg("    Log MAIL FROM Address: %s\n",
                config->log_mailfrom ? "Enabled" : "Not Enabled");

    _dpd.logMsg("    Log RCPT TO Addresses: %s\n",
                config->log_rcptto ? "Enabled" : "Not Enabled");

    _dpd.logMsg("    Log Email Headers: %s\n",
                config->log_email_hdrs ? "Enabled" : "Not Enabled");

    if(config->log_email_hdrs)
    {
        _dpd.logMsg("    Email Hdrs Log Depth: %u\n",
                config->email_hdrs_log_depth);
    }
}

/*
**  NAME
**    ProcessPorts::
*/
/**
**  Process the port list.
**
**  This configuration is a list of valid ports and is ended by a
**  delimiter.
**
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessPorts(SMTPConfig *config, char *ErrorString, int ErrStrLen)
{
    char *pcToken;
    char *pcEnd;
    int  iPort;
    int  iEndPorts = 0;
    int num_ports = 0;

    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    pcToken = strtok(NULL, CONF_SEPARATORS);
    if(!pcToken)
    {
        snprintf(ErrorString, ErrStrLen, "Invalid port list format.");
        return -1;
    }

    if(strcmp(CONF_START_LIST, pcToken))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start a port list with the '%s' token.", CONF_START_LIST);

        return -1;
    }

    /* Since ports are specified, clear default ports */
    config->ports[SMTP_DEFAULT_SERVER_PORT / 8] &= ~(1 << (SMTP_DEFAULT_SERVER_PORT % 8));
    config->ports[XLINK2STATE_DEFAULT_PORT / 8] &= ~(1 << (XLINK2STATE_DEFAULT_PORT % 8));
    config->ports[SMTP_DEFAULT_SUBMISSION_PORT / 8] &= ~(1 << (SMTP_DEFAULT_SUBMISSION_PORT % 8));

    while ((pcToken = strtok(NULL, CONF_SEPARATORS)) != NULL)
    {
        if(!strcmp(CONF_END_LIST, pcToken))
        {
            iEndPorts = 1;
            break;
        }

        iPort = strtol(pcToken, &pcEnd, 10);

        /*
        **  Validity check for port
        */
        if(*pcEnd)
        {
            snprintf(ErrorString, ErrStrLen,
                     "Invalid port number.");

            return -1;
        }

        if(iPort < 0 || iPort > MAXPORTS-1)
        {
            snprintf(ErrorString, ErrStrLen,
                     "Invalid port number.  Must be between 0 and 65535.");

            return -1;
        }

        config->ports[iPort / 8] |= (1 << (iPort % 8));
        num_ports++;
    }

    if(!iEndPorts)
    {
        snprintf(ErrorString, ErrStrLen,
                 "Must end '%s' configuration with '%s'.",
                 CONF_PORTS, CONF_END_LIST);

        return -1;
    }
    else if(!num_ports)
    {
        snprintf(ErrorString, ErrStrLen,
             "SMTP: Empty port list not allowed.");
        return -1;
    }

    return 0;
}

/*
**  NAME
**    ProcessCmds::
*/
/**
**  Process the command list.
**
**  This configuration is a list of valid ports and is ended by a
**  delimiter.
**
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
*/
static int ProcessCmds(SMTPConfig *config, char *ErrorString, int ErrStrLen, int action)
{
    char *pcToken;
    int   iEndCmds = 0;
    int   id;

    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    pcToken = strtok(NULL, CONF_SEPARATORS);
    if (!pcToken)
    {
        snprintf(ErrorString, ErrStrLen, "Invalid command list format.");
        return -1;
    }

    if (strcmp(CONF_START_LIST, pcToken))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start a command list with the '%s' token.",
                CONF_START_LIST);

        return -1;
    }

    while ((pcToken = strtok(NULL, CONF_SEPARATORS)) != NULL)
    {
        if (strcmp(CONF_END_LIST, pcToken) == 0)
        {
            iEndCmds = 1;
            break;
        }

        id = GetCmdId(config, pcToken);

        if (action == ACTION_ALERT)
        {
            config->cmd_config[id].alert = 1;
        }
        else if (action == ACTION_NO_ALERT)
        {
            config->cmd_config[id].alert = 0;
        }
        else if (action == ACTION_NORMALIZE)
        {
            config->cmd_config[id].normalize = 1;
        }
    }

    if (!iEndCmds)
    {
        snprintf(ErrorString, ErrStrLen, "Must end '%s' configuration with '%s'.",
                 action == ACTION_ALERT ? CONF_INVALID_CMDS :
                 (action == ACTION_NO_ALERT ? CONF_VALID_CMDS :
                  (action == ACTION_NORMALIZE ? CONF_NORMALIZE_CMDS : "")),
                 CONF_END_LIST);

        return -1;
    }

    return 0;
}

/* Return id associated with a given command string */
static int GetCmdId(SMTPConfig *config, char *name)
{
    const SMTPToken *cmd;

    for (cmd = config->cmds; cmd->name != NULL; cmd++)
    {
        if (strcasecmp(cmd->name, name) == 0)
        {
            return cmd->search_id;
        }
    }

    return AddCmd(config, name);
}


static int AddCmd(SMTPConfig *config, char *name)
{
    SMTPToken *cmds, *tmp_cmds;
    SMTPSearch *cmd_search;
    SMTPCmdConfig *cmd_config;
    int ret;

    if (config == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) SMTP config is NULL.\n",
                                        __FILE__, __LINE__);
    }

    config->num_cmds++;

    /* allocate enough memory for new commmand - alloc one extra for NULL entry */
    cmds = (SMTPToken *)calloc(config->num_cmds + 1, sizeof(SMTPToken));
    if (cmds == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory for SMTP "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    /* This gets filled in later */
    cmd_search = (SMTPSearch *)calloc(config->num_cmds, sizeof(SMTPSearch));
    if (cmd_search == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory for SMTP "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    cmd_config = (SMTPCmdConfig *)calloc(config->num_cmds, sizeof(SMTPCmdConfig));
    if (cmd_config == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory for SMTP "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    /* copy existing commands into newly allocated memory
     * don't need to copy anything from cmd_search since this hasn't been initialized yet */
    ret = SafeMemcpy(cmds, config->cmds, (config->num_cmds - 1) * sizeof(SMTPToken),
                     cmds, cmds + (config->num_cmds - 1));

    if (ret != SAFEMEM_SUCCESS)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to memory copy SMTP command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    ret = SafeMemcpy(cmd_config, config->cmd_config, (config->num_cmds - 1) * sizeof(SMTPCmdConfig),
                     cmd_config, cmd_config + (config->num_cmds - 1));

    if (ret != SAFEMEM_SUCCESS)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to memory copy SMTP command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    /* add new command to cmds
     * cmd_config doesn't need anything added - this will probably be done by a calling function
     * cmd_search will be initialized when the searches are initialized */
    tmp_cmds = &cmds[config->num_cmds - 1];
    tmp_cmds->name = strdup(name);
    tmp_cmds->name_len = strlen(name);
    tmp_cmds->search_id = config->num_cmds - 1;

    if (tmp_cmds->name == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory for SMTP "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    /* free global memory structures */
    if (config->cmds != NULL)
        free(config->cmds);

    if (config->cmd_search != NULL)
        free(config->cmd_search);

    if (config->cmd_config != NULL)
        free(config->cmd_config);

    /* set globals to new memory */
    config->cmds = cmds;
    config->cmd_search = cmd_search;
    config->cmd_config = cmd_config;

    return (config->num_cmds - 1);
}

static int ProcessMaxMimeMem(SMTPConfig *config, char *ErrorString, int ErrStrLen)
{
    char *endptr;
    char *value;
    int max_mime_mem = 0;
    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    value = strtok(NULL, CONF_SEPARATORS);
    if ( value == NULL )
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid format for max_mime_mem.");
        return -1;
    }
    max_mime_mem = strtol(value, &endptr, 10);

    if(*endptr)
    {
        snprintf(ErrorString, ErrStrLen,
            "Invalid format for max_mime_mem.");
        return -1;
    }

    if (max_mime_mem < MIN_MIME_MEM || max_mime_mem > MAX_MIME_MEM)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid value for max_mime_mem."
                "It should range between %d and %d.",
                MIN_MIME_MEM, MAX_MIME_MEM);
        return -1;
    }

    config->max_mime_mem = max_mime_mem;
    return 0;
}


static int ProcessSmtpMemcap(SMTPConfig *config, char *ErrorString, int ErrStrLen)
{
    char *endptr;
    char *value;
    uint32_t smtp_memcap = 0;
    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    value = strtok(NULL, CONF_SEPARATORS);
    if ( value == NULL )
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid format for memcap.");
        return -1;
    }
    smtp_memcap = strtoul(value, &endptr, 10);

    if((value[0] == '-') || (*endptr != '\0'))
    {
        snprintf(ErrorString, ErrStrLen,
            "Invalid format for memcap.");
        return -1;
    }

    if (smtp_memcap < MIN_SMTP_MEMCAP || smtp_memcap > MAX_SMTP_MEMCAP)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid value for memcap."
                "It should range between %d and %d.",
                MIN_SMTP_MEMCAP, MAX_SMTP_MEMCAP);
        return -1;
    }

    config->memcap = smtp_memcap;
    return 0;
}


static int ProcessMaxMimeDepth(SMTPConfig *config, char *ErrorString, int ErrStrLen)
{
    char *endptr;
    char *value;
    int max_mime_depth = 0;
    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    value = strtok(NULL, CONF_SEPARATORS);
    if ( value == NULL )
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid format for max_mime_depth.");
        return -1;
    }
    max_mime_depth = strtol(value, &endptr, 10);

    if(*endptr)
    {
        snprintf(ErrorString, ErrStrLen,
            "Invalid format for max_mime_depth.");
        return -1;
    }

    if (max_mime_depth < MIN_MIME_DEPTH || max_mime_depth > MAX_MIME_DEPTH)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid value for max_mime_depth."
                "It should range between %d and %d.",
                MIN_MIME_DEPTH, MAX_MIME_DEPTH);
        return -1;
    }
    if(max_mime_depth & 3)
    {
        max_mime_depth += 4 - (max_mime_depth & 3);
        _dpd.logMsg("WARNING: %s(%d) => SMTP: 'max_mime_depth' is not a multiple of 4. "
             "Rounding up to the next multiple of 4. The new 'max_mime_depth' is %d.\n",
              *(_dpd.config_file), *(_dpd.config_line), max_mime_depth);

    }

    config->max_mime_depth = max_mime_depth;
    return 0;
}

static int ProcessLogDepth(SMTPConfig *config, char *ErrorString, int ErrStrLen)
{
    char *endptr;
    char *value;
    uint32_t log_depth = 0;
    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    value = strtok(NULL, CONF_SEPARATORS);
    if ( value == NULL )
    {
        snprintf(ErrorString, ErrStrLen,
                "Missing value for email_hdrs_log_depth.");
        return -1;
    }
    log_depth = strtoul(value, &endptr, 10);

    if((value[0] == '-') || (*endptr != '\0'))
    {
        snprintf(ErrorString, ErrStrLen,
            "Invalid format '%s' for email_hdrs_log_depth.",
            value);
        return -1;
    }

    if(log_depth && log_depth < MIN_LOG_DEPTH)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid value for email_hdrs_log_depth."
                "It should range between %d and %d.",
                MIN_LOG_DEPTH, MAX_LOG_DEPTH);
        return -1;
    }
    else if (log_depth > MAX_LOG_DEPTH)
    {
        _dpd.logMsg("WARNING: %s(%d) => Invalid value for email_hdrs_log_depth. "
                "It should range between %d and %d. The email_hdrs_log_depth "
                "will be reduced to the max value.\n", *(_dpd.config_file), *(_dpd.config_line),
                MIN_LOG_DEPTH, MAX_LOG_DEPTH);

        log_depth = MAX_LOG_DEPTH;
    }


    config->email_hdrs_log_depth = log_depth;
    return 0;
}

static int ProcessDecodeDepth(SMTPConfig *config, char *ErrorString, int ErrStrLen, char *decode_type, DecodeType type)
{
    char *endptr;
    char *value;
    int decode_depth = 0;
    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    value = strtok(NULL, CONF_SEPARATORS);
    if ( value == NULL )
    {
        snprintf(ErrorString, ErrStrLen,
            "Invalid format for SMTP config option '%s'.", decode_type);
        return -1;
    }
    decode_depth = strtol(value, &endptr, 10);

    if(*endptr)
    {
        snprintf(ErrorString, ErrStrLen,
            "Invalid format for SMTP config option '%s'.", decode_type);
        return -1;
    }
    if(decode_depth < MIN_DEPTH || decode_depth > MAX_DEPTH)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid value for SMTP config option '%s'."
                "It should range between %d and %d.",
                decode_type, MIN_DEPTH, MAX_DEPTH);
        return -1;
    }

    switch(type)
    {
        case DECODE_B64:
                if((decode_depth > 0) && (decode_depth & 3))
                {
                    decode_depth += 4 - (decode_depth & 3);
                    if(decode_depth > MAX_DEPTH )
                    {
                        decode_depth = decode_depth - 4;
                    }
                    _dpd.logMsg("WARNING: %s(%d) => SMTP: 'b64_decode_depth' is not a multiple of 4. "
                            "Rounding up to the next multiple of 4. The new 'b64_decode_depth' is %d.\n",
                            *(_dpd.config_file), *(_dpd.config_line), decode_depth);
                }
                config->b64_depth = decode_depth;
                break;
        case DECODE_QP:
                config->qp_depth = decode_depth;
                break;
        case DECODE_UU:
                config->uu_depth = decode_depth;
                break;
        case DECODE_BITENC:
                config->bitenc_depth = decode_depth;
                break;
        default:
                return -1;
    }

    return 0;
}



/*
**  NAME
**    ProcessAltMaxCmdLen::
*/
/**
**
**   alt_max_command_line_len <int> { <cmd> [<cmd>] }
**
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
*/
static int ProcessAltMaxCmdLen(SMTPConfig *config, char *ErrorString, int ErrStrLen)
{
    char *pcToken;
    char *pcLen;
    char *pcLenEnd;
    int   iEndCmds = 0;
    int   id;
    int   cmd_len;

    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    /* Find number */
    pcLen = strtok(NULL, CONF_SEPARATORS);
    if (!pcLen)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid format for alt_max_command_line_len.");

        return -1;
    }

    pcToken = strtok(NULL, CONF_SEPARATORS);
    if (!pcToken)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid format for alt_max_command_line_len.");

        return -1;
    }

    cmd_len = strtoul(pcLen, &pcLenEnd, 10);
    if (pcLenEnd == pcLen)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid format for alt_max_command_line_len (non-numeric).");

        return -1;
    }

    if (strcmp(CONF_START_LIST, pcToken))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start alt_max_command_line_len list with the '%s' token.",
                CONF_START_LIST);

        return -1;
    }

    while ((pcToken = strtok(NULL, CONF_SEPARATORS)) != NULL)
    {
        if (strcmp(CONF_END_LIST, pcToken) == 0)
        {
            iEndCmds = 1;
            break;
        }

        id = GetCmdId(config, pcToken);

        config->cmd_config[id].max_line_len = cmd_len;
    }

    if (!iEndCmds)
    {
        snprintf(ErrorString, ErrStrLen,
                "Must end alt_max_command_line_len configuration with '%s'.", CONF_END_LIST);

        return -1;
    }

    return 0;
}


/*
**  NAME
**    ProcessXlink2State::
*/
/**
**
**   xlink2state { <enable/disable> <drop> }
**
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
*/
static int ProcessXlink2State(SMTPConfig *config, char *ErrorString, int ErrStrLen)
{
    char *pcToken;
    int  iEnd = 0;

    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    pcToken = strtok(NULL, CONF_SEPARATORS);
    if(!pcToken)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid xlink2state argument format.");

        return -1;
    }

    if(strcmp(CONF_START_LIST, pcToken))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start xlink2state arguments with the '%s' token.",
                CONF_START_LIST);

        return -1;
    }

    while ((pcToken = strtok(NULL, CONF_SEPARATORS)) != NULL)
    {
        if(!strcmp(CONF_END_LIST, pcToken))
        {
            iEnd = 1;
            break;
        }

        if ( !strcasecmp(CONF_DISABLE, pcToken) )
        {
            config->alert_xlink2state = 0;
            config->ports[XLINK2STATE_DEFAULT_PORT / 8] &= ~(1 << (XLINK2STATE_DEFAULT_PORT % 8));
        }
        else if ( !strcasecmp(CONF_ENABLE, pcToken) )
        {
            config->alert_xlink2state = 1;
            config->ports[XLINK2STATE_DEFAULT_PORT / 8] |= 1 << (XLINK2STATE_DEFAULT_PORT % 8);
        }
        else if ( !strcasecmp(CONF_INLINE_DROP, pcToken) )
        {
            if (!config->alert_xlink2state)
            {
                snprintf(ErrorString, ErrStrLen,
                         "Alerting on X-LINK2STATE must be enabled to drop.");

                return -1;
            }
            config->drop_xlink2state = 1;
        }
    }

    if(!iEnd)
    {
        snprintf(ErrorString, ErrStrLen,
                "Must end '%s' configuration with '%s'.",
                CONF_XLINK2STATE, CONF_END_LIST);

        return -1;
    }

    return 0;
}

