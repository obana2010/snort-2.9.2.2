/*
** Copyright (C) 2003-2012 Sourcefire, Inc.
** 
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


#ifndef __SP_FILE_DATA_H__
#define __SP_FILE_DATA_H__

typedef struct _FileData
{
    uint8_t mime_decode_flag;
}FileData;

void SetupFileData(void);
int FileDataCompare(void *, void *);
uint32_t FileDataHash(void *);

#endif  
