/*
 ** Copyright (C) 1998-2012 Sourcefire, Inc.
 **
 ** Writen by Patrick Mullen <pmullen@sourcefire.com>
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

#ifndef _SF_BASE64DECODE_H_
#define _SF_BASE64DECODE_H_

#include "sf_types.h"
#include "util_unfold.h"

int sf_base64decode(uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t*); 

#endif
