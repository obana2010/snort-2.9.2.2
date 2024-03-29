/*
 ** Copyright (C) 1998-2012 Sourcefire, Inc.
 **
 ** Writen by Bhagyashree Bantwal <bbantwal@sourcefire.com>
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

#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<ctype.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define ALERT_SPACES_EXCEEDED   0x1
#define ALERT_LEVELS_EXCEEDED   0x2
#define ALERT_MIXED_ENCODINGS 0x4

#define MAX_ALLOWED_OBFUSCATION 1

typedef struct {
    int allowed_spaces;
    int allowed_levels;
    uint16_t alerts;
}JSState;

int JSNormalizeDecode(char *, uint16_t , char *, uint16_t destlen, char **, int *, JSState *, int *);
void InitJSNormLookupTable(void);
