/**
 * \file tuya_ble_utils.h
 *
 * \brief 
 */
/*
 *  Copyright (C) 2014-2019, Tuya Inc., All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of tuya ble sdk 
 */

#ifndef TUYA_BLE_UTILS_H_
#define TUYA_BLE_UTILS_H_

#include "tuya_ble_stdlib.h"



void tuya_ble_inverted_array(uint8_t *array,uint16_t length);

bool tuya_ble_buffer_value_is_all_x(uint8_t *buffer,uint16_t len,uint8_t value);

uint8_t tuya_ble_check_sum(uint8_t *pbuf,uint16_t len);

uint8_t tuya_ble_check_num(uint8_t *buf,uint8_t num);

void tuya_ble_hextoascii(uint8_t *hexbuf,uint8_t len,uint8_t *ascbuf);

void tuya_ble_asciitohex(uint8_t *ascbuf,uint8_t *hexbuf);

uint8_t tuya_ble_char_2_ascii(uint8_t data);

void tuya_ble_str_to_hex(uint8_t *str_buf,uint8_t str_len,uint8_t *hex_buf);

bool tuya_ble_is_word_aligned_tuya(void const* p);

uint16_t tuya_ble_crc16_compute(uint8_t * p_data, uint16_t size, uint16_t * p_crc);

uint32_t tuya_ble_crc32_compute(uint8_t const * p_data, uint32_t size, uint32_t const * p_crc);

void tuya_ble_device_id_20_to_16(uint8_t *in,uint8_t *out);

void tuya_ble_device_id_16_to_20(uint8_t *in,uint8_t *out);

#endif /* TUYA_UTILS_H_ */
