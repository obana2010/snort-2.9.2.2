/*
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
 * Rule options for Modbus preprocessor.
 *
 */

#ifndef MODBUS_ROPTIONS_H
#define MODBUS_ROPTIONS_H

#include <stdint.h>

#define MODBUS_FUNC_NAME "modbus_func"
#define MODBUS_UNIT_NAME "modbus_unit"
#define MODBUS_DATA_NAME "modbus_data"

/* Data types */
typedef enum _modbus_option_type_t
{
    MODBUS_FUNC = 0,
    MODBUS_UNIT,
    MODBUS_DATA
} modbus_option_type_t;

typedef struct _modbus_option_data_t
{
    modbus_option_type_t type;
    uint16_t arg;
} modbus_option_data_t;

typedef struct _modbus_func_map_t
{
    char *name;
    uint8_t func;
} modbus_func_map_t;

int ModbusFuncInit(char *name, char *params, void **data);
int ModbusUnitInit(char *name, char *params, void **data);
int ModbusDataInit(char *name, char *params, void **data);

int ModbusRuleEval(void *raw_packet, const uint8_t **cursor, void *data);

#endif /* MODBUS_ROPTIONS_H */
