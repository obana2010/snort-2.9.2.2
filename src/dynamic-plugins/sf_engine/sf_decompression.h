/*
 * sf_decompression.h
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
 * Copyright (C) 2011-2012 Sourcefire, Inc.
 *
 * Author: Ryan Jordan
 *
 * Date: 3/8/2011
 *
 * Decompression API for Snort Plugins.
 *
 */

#ifndef SF_DECOMPRESSION_H
#define SF_DECOMPRESSION_H

#include <stdint.h>
#include "sf_snort_plugin_api.h"

/* Types */
typedef enum compression_type
{
    COMPRESSION_TYPE_DEFLATE = 1,
    COMPRESSION_TYPE_GZIP,
    COMPRESSION_TYPE_MAX
} compression_type_t;

/* Return Codes */
#define SNORT_DECOMPRESS_OK 0
#define SNORT_DECOMPRESS_OUTPUT_TRUNC  1
#define SNORT_DECOMPRESS_BAD_ARGUMENT (-1)
#define SNORT_DECOMPRESS_BAD_DATA     (-2)
#define SNORT_DECOMPRESS_ERROR (-3)


/* API Functions */

/* This function initializes a Decompression API state object.
   It must be called first when using decompression.

   Arguments: type => Type of decompression to use (gzip, deflate)
   Returns: void pointer to decompression state object
*/
ENGINE_LINKAGE void * SnortDecompressInit(compression_type_t type);

/* This function destroys a Decompression API state object.

   Arguments: void *s => state object allocated by SnortDecompressInit().
   Returns: SNORT_DECOMPRESS_OK on success, negative on error.
*/
ENGINE_LINKAGE int SnortDecompressDestroy(void *state);

/* This is the function that decompresses data.

   Arguments:
     void *state             => pointer to state object allocated by SnortDecompressInit().
     uint8_t *input          => pointer to buffer that stores compressed data.
                                pass NULL to continue decompressing the previous input.
     uint32_t input_len      => length of input to decompress.
                                ignored if "input" is set to NULL.
     uint8_t *output         => pointer to buffer where decompressed output will be stored.
     uint32_t output_bufsize => available space in output buffer.
     uint32_t *output_len    => gets set to the actual amount of output generated.
   Returns:
     SNORT_DECOMPRESS_OK:           success
     SNORT_DECOMPRESS_BAD_ARGUMENT: Bad arguments passed in. Could be null pointers,
                                    uninitialized state objects.
     SNORT_DECOMPRESS_BAD_DATA:     Error decompressing the data. Could be corrupted
                                    input, or the wrong compression type was set.
     SNORT_DECOMPRESS_OUTPUT_TRUNC: Decompression was successful, but the output
                                    buffer filled up. Call SnortDecompress() again
                                    with NULL input after consuming the output.
*/
ENGINE_LINKAGE int SnortDecompress(void *state, uint8_t *input, uint32_t input_len, 
                    uint8_t *output, uint32_t output_bufsize, uint32_t *output_len);


#endif /* SF_DECOMPRESSION_H */
