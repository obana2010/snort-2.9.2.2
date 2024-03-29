/*
 ** Copyright (C) 2011-2012 Sourcefire, Inc.
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
 **
 ** 9/7/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 */

#ifndef SFRT_DIR_FLAT_H_
#define SFRT_DIR_FLAT_H_

#include "sfrt_flat.h"
#include "sfrt.h"

typedef MEM_OFFSET SUB_TABLE_PTR;
typedef MEM_OFFSET ENTRIES_PTR;

typedef struct
{
    MEM_OFFSET value;
    uint8_t length;

}DIR_Entry;

/*******************************************************************/
/* DIR-n-m data structures
 * Each table in the DIR-n-m method is represented by a
 * dir_sub_table_t.  They are managed by a dir_table_t. */
typedef struct
{
    int num_entries; /* Number of entries in this table */
    int width;       /* width of this table. */
                     /* While one determines the other, this way fewer
                      * calculations are needed at runtime, since both
                      * are used. */
    int cur_num;     /* Present number of used nodes */

    ENTRIES_PTR entries;

} dir_sub_table_flat_t;

/* Master data structure for the DIR-n-m derivative */
typedef struct
{
    int dimensions[20];    /* DIR-n-m will consist of any number of arbitrarily
                         * long tables. This variable keeps track of the
                         * dimensions */
    int dim_size;       /* And this variable keeps track of 'dimensions''s
                         * dimensions! */
    uint32_t mem_cap;  /* User-defined maximum memory that can be allocated
                         * for the DIR-n-m derivative */

    int cur_num;        /* Present number of used nodes */

    uint32_t allocated;

    SUB_TABLE_PTR sub_table;
} dir_table_flat_t;

/*******************************************************************/
/* DIR-n-m functions, these are not intended to be called directly */
TABLE_PTR sfrt_dir_flat_new(uint32_t mem_cap, int count,...);
void         sfrt_dir_flat_free(TABLE_PTR);
tuple_flat_t  sfrt_dir_flat_lookup(snort_ip_p ip, TABLE_PTR table);
int           sfrt_dir_flat_insert(snort_ip_p ip, int len, word data_index,
                               int behavior, TABLE_PTR);
uint32_t      sfrt_dir_flat_usage(TABLE_PTR);

#endif /* SFRT_DIR_FLAT_H_ */

