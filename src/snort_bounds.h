#ifndef _BOUNDS_H
#define _BOUNDS_H
/*
** Copyright (C) 2003-2012 Sourcefire, Inc.
**               Chris Green <cmg@sourcefire.com>
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
*/

#ifdef OSF1
#include <sys/bitypes.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef DEBUG
#include <assert.h>
#endif
#include <unistd.h>

#define SAFEMEM_ERROR 0
#define SAFEMEM_SUCCESS 1

#ifdef DEBUG
#define ERRORRET assert(0==1)
#else
#define ERRORRET return SAFEMEM_ERROR;
#endif /* DEBUG */

#define MAXPORTS 65536
#define MAXPORTS_STORAGE 8192


/*
 * Check to make sure that p is less than or equal to the ptr range
 * pointers
 *
 * 1 means it's in bounds, 0 means it's not
 */
static inline int inBounds(const uint8_t *start, const uint8_t *end, const uint8_t *p)
{
    if ((p >= start) && (p < end))
        return 1;
    return 0;
}

static inline int SafeMemCheck(void *dst, size_t n,
        const void *start, const void *end)
{
    void *tmp;

    if (n < 1)
        return SAFEMEM_ERROR;

    if ((dst == NULL) || (start == NULL) || (end == NULL))
        return SAFEMEM_ERROR;

    tmp = ((uint8_t *)dst) + (n - 1);
    if (tmp < dst)
        return SAFEMEM_ERROR;

    if (!inBounds(start, end, dst) || !inBounds(start, end, tmp))
        return SAFEMEM_ERROR;

    return SAFEMEM_SUCCESS;
}

/**
 * A Safer Memcpy
 *
 * @param dst where to copy to
 * @param src where to copy from
 * @param n number of bytes to copy
 * @param start start of the dest buffer
 * @param end end of the dst buffer
 *
 * @return SAFEMEM_ERROR on failure, SAFEMEM_SUCCESS on success
 */
static inline int SafeMemcpy(void *dst, const void *src, size_t n, const void *start, const void *end)
{
    if (SafeMemCheck(dst, n, start, end) != SAFEMEM_SUCCESS)
        ERRORRET;
    if (src == NULL)
        ERRORRET;
    memcpy(dst, src, n);
    return SAFEMEM_SUCCESS;
}

/**
 * A Safer Memmove
 * dst and src can be in the same buffer
 *
 * @param dst where to copy to
 * @param src where to copy from
 * @param n number of bytes to copy
 * @param start start of the dest buffer
 * @param end end of the dst buffer
 *
 * @return SAFEMEM_ERROR on failure, SAFEMEM_SUCCESS on success
 */
static inline int SafeMemmove(void *dst, const void *src, size_t n, const void *start, const void *end)
{
    if (SafeMemCheck(dst, n, start, end) != SAFEMEM_SUCCESS)
        ERRORRET;
    if (src == NULL)
        ERRORRET;
    memmove(dst, src, n);
    return SAFEMEM_SUCCESS;
}

/**
 * A Safer Memmove
 * dst and src can be in the same buffer
 *
 * @param dst where to copy to
 * @param src where to copy from
 * @param n number of bytes to copy
 * @param start start of the dest buffer
 * @param end end of the dst buffer
 *
 * @return SAFEMEM_ERROR on failure, SAFEMEM_SUCCESS on success
 */
static inline int SafeBoundsMemmove(void *dst, const void *src, size_t n, const void *start, const void *end)
{
    size_t overlap = 0;
    if (SafeMemCheck(dst, n, start, end) != SAFEMEM_SUCCESS)
        ERRORRET;
    if (src == NULL)
        ERRORRET;

    if( src == dst )
    {
        return SAFEMEM_SUCCESS;
    }
    else if(inBounds(dst, ((uint8_t *)dst + n), src))
    {
        overlap = (uint8_t *)src - (uint8_t *)dst;
        memcpy(dst, src , overlap);
        memmove(((uint8_t *)dst + overlap), ((uint8_t *)src + overlap), (n - overlap));
    }
    else if(inBounds(src, ((uint8_t *)src + n), dst))
    {
        overlap = (uint8_t *)dst - (uint8_t *)src;
        memcpy(((uint8_t *)dst + overlap), ((uint8_t *)src + overlap), (n - overlap));
        memmove(dst, src, overlap);
    }
    else
    {
        memcpy(dst, src, n);
    }
    return SAFEMEM_SUCCESS;
}
/**
 * A Safer Memset
 * dst and src can be in the same buffer
 *
 * @param dst where to copy to
 * @param c character to set memory with
 * @param n number of bytes to set
 * @param start start of the dst buffer
 * @param end end of the dst buffer
 *
 * @return SAFEMEM_ERROR on failure, SAFEMEM_SUCCESS on success
 */
static inline int SafeMemset(void *dst, uint8_t c, size_t n, const void *start, const void *end)
{
    if (SafeMemCheck(dst, n, start, end) != SAFEMEM_SUCCESS)
        ERRORRET;
    memset(dst, c, n);
    return SAFEMEM_SUCCESS;
}

/**
 * A Safer *a = *b
 *
 * @param start start of the dst buffer
 * @param end end of the dst buffer
 * @param dst the location to write to
 * @param src the source to read from
 *
 * @return 0 on failure, 1 on success
 */
static inline int SafeWrite(uint8_t *start, uint8_t *end, uint8_t *dst, uint8_t *src)
{
    if(!inBounds(start, end, dst))
    {
        ERRORRET;
    }

    *dst = *src;
    return 1;
}

static inline int SafeRead(uint8_t *start, uint8_t *end, uint8_t *src, uint8_t *read)
{
    if(!inBounds(start,end, src))
    {
        ERRORRET;
    }

    *read = *start;
    return 1;
}

#endif /* _BOUNDS_H */
