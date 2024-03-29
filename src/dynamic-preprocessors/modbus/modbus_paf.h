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
 * Protocol-Aware Flushing (PAF) code for the Modbus preprocessor.
 *
 */

#ifndef MODBUS_PAF__H
#define MODBUS_PAF__H

#include "spp_modbus.h"
#include "stream_api.h"

typedef enum _modbus_paf_state
{
    MODBUS_PAF_STATE__TRANS_ID_1 = 0,
    MODBUS_PAF_STATE__TRANS_ID_2,
    MODBUS_PAF_STATE__PROTO_ID_1,
    MODBUS_PAF_STATE__PROTO_ID_2,
    MODBUS_PAF_STATE__LENGTH_1,
    MODBUS_PAF_STATE__LENGTH_2,
    MODBUS_PAF_STATE__SET_FLUSH
} modbus_paf_state_t;

typedef struct _modbus_paf_data
{
    modbus_paf_state_t state;
    uint16_t modbus_length;
} modbus_paf_data_t;

void ModbusAddPortsToPaf(modbus_config_t *config, tSfPolicyId policy_id);
int ModbusPafRegister(uint16_t port, tSfPolicyId policy_id);
PAF_Status ModbusPaf(void *ssn, void **user, const uint8_t *data, 
                     uint32_t len, uint32_t flags, uint32_t *fp);

#endif /* MODBUS_PAF__H */
