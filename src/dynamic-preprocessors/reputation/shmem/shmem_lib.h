/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2011-2012 Sourcefire, Inc.
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

// @file    shmem_lib.h
// @author  Pramod Chandrashekar <pramod@sourcefire.com>

#ifndef _SHMEMLIB_H_
#define _SHMEMLIB_H_

#include <stdint.h>

int   ShmemExists(const char *shmemName);
void* ShmemMap(const char* segment_name, uint32_t size, int mode);
void  ShmemUnlink(const char *shmemName);
void  ShmemDestroy(const char *shmemName);

#endif
