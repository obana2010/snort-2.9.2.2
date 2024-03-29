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

// @file    shmem_config.h
// @author  Pramod Chandrashekar <pramod@sourcefire.com>

#ifndef _SHMEMCFG_H_
#define _SHMEMCFG_H_

#include <stdint.h>

#include "shmem_datamgmt.h"  //defines shmemdata filelist
#include "shmem_common.h"  

#define SHMEM_MGMT   "SFShmemMgmt"

#define MAX_SEGMENTS    2
#define MAX_INSTANCES  50

#define WRITE           0 
#define READ            1

#define SERVER          0
#define CLIENT1         1
#define CLIENT2         2

#define STARTUP         1 
#define RELOAD          0

#define ACTIVE          1
#define INACTIVE        0

#define NO_DATASEG     -1
#define NO_ZEROSEG     -2
#define UNMAP_OLDSEG   -3
#define NO_FILE        -4
#define ZEROSEG       100

#define NUMA_0          0
#define NUMA_1          1

#define GROUP_0         0

#define SLEEP_TIME      2  // in micro seconds

#define TBMAP                 99
#define UNUSED_TIMEOUT        10 //this number is multiplied with outofband check time to determine timeout.
#define OUT_OF_BAND_CHEK_TIME 10 

typedef struct shmemUserInfo {
    uint32_t  instance_num;   //unique ID for each snort instance      
    int       instance_type;  // READ or WRITE
    int       dataset;        // IPRep
    int       group_id;       // 0,1...
    int       numa_node;
    char      mgmtSeg[MAX_NAME];
    char      dataSeg[MAX_SEGMENTS][MAX_NAME];
    char      path[MAX_NAME];
    uint32_t  instance_polltime;
}ShmemUserInfo;

typedef struct
{
    const char *const name;
    const uint32_t type;
} DatasetInfo;

typedef struct shmemDataManagmentFunctions {
    int (*CreatePerProcessZeroSegment)(void*** data_ptr);
    uint32_t (*GetSegmentSize)(ShmemDataFileList** file_list, int file_count);
    int (*LoadShmemData)(void* data_ptr, ShmemDataFileList** file_list, int file_count);
} ShmemDataMgmtFunctions;

typedef int      (*CreateMallocZero)(void***);
typedef uint32_t (*GetDataSize)(ShmemDataFileList**, int);
typedef int      (*LoadData)(void*,ShmemDataFileList**,int);

extern ShmemDataMgmtFunctions *dmfunc_ptr; 
extern ShmemUserInfo *shmusr_ptr;

void PrintConfig(void);

int InitShmemUser(
   uint32_t instance_num, int instance_type, int dataset, int group_id,
   int numa_node, const char* path, uint32_t instance_polltime);

int InitShmemDataMgmtFunctions(
    CreateMallocZero create_malloc_zero, GetDataSize get_data_size,
    LoadData load_data);

void FreeShmemUser(void);
void FreeShmemDataMgmtFunctions(void);
#endif

