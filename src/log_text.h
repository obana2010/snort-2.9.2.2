
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
 
/**
 * @file   log_text.h
 * @author Russ Combs <rcombs@sourcefire.com>
 * @date   Fri Jun 27 10:34:37 2003
 * 
 * @brief  logging to text file
 * 
 * Use these methods to write to a TextLog.
 */

#ifndef _LOG_TEXT_H
#define _LOG_TEXT_H

#include "sfutil/sf_textlog.h"

void LogPriorityData(TextLog*, bool doNewLine);
void LogXrefs(TextLog*, bool doNewLine);

void LogIPPkt(TextLog*, int type, Packet*);

void LogTimeStamp(TextLog*, Packet*);
void LogTrHeader(TextLog*, Packet*);
void Log2ndHeader(TextLog*, Packet*);
void LogIpAddrs(TextLog*, Packet*);
void LogIPHeader(TextLog*, Packet*);
void LogTCPHeader(TextLog*, Packet*);
void LogUDPHeader(TextLog*, Packet*);
void LogICMPHeader(TextLog*, Packet*);
void LogArpHeader(TextLog*, Packet*);

#if 0
/* these are implemented in log_text.c but not public */
static void LogEthHeader(TextLog*, Packet*);
static void LogSLLHeader(TextLog*, Packet*);
static void LogWifiHeader(TextLog*, Packet*);
static void LogIpOptions(TextLog* , Packet*);
static void LogTcpOptions(TextLog*, Packet*);
static void LogEmbeddedICMPHeader(TextLog*, const ICMPHdr*);
static void LogICMPEmbeddedIP(TextLog*, Packet*);
static void LogReference(TextLog*, ReferenceNode*);
static void ScOutputCharData(TextLog*, char* data, int len);
static void LogNetData (TextLog*, const u_char* data, const int len, Packet *);
#endif

#if 0
/* these are only in log.c: */
/* called from snort.c: */
void PrintEapolPkt(FILE*, Packet*);
/* *Key() and *Header() should be static/private */
void PrintEapolKey(FILE*, Packet*);
void PrintEapolHeader(FILE*, Packet*);
void PrintEAPHeader(FILE*, Packet*);

/* commented out all over the place! */
/* still called in snort.c and spp_stream4.c */
void ClearDumpBuf(void);

/* called from snort.c */
void PrintWifiPkt(FILE*, Packet*);

/* called in a few places including log_text.c */
void CreateTCPFlagString(Packet*, char*);
#endif

#endif /* _LOG_TEXT_H */

