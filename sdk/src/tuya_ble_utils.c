/**
 * \file tuya_ble_utils.c
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
#include "tuya_ble_stdlib.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_port.h"
#include "tuya_ble_mem.h"


void tuya_ble_inverted_array(uint8_t *array,uint16_t length)
{
    uint8_t temp;
    uint16_t i,j;
    for (i = 0,j=length-1; i<j; i++,j--)
    {
        temp=array[i];
        array[i]=array[j];
        array[j]=temp;
    }
}

bool tuya_ble_buffer_value_is_all_x(uint8_t *buffer,uint16_t len,uint8_t value)
{
    bool ret = true;
    for(uint16_t i = 0; i<len; i++)
    {
        if(buffer[i]!= value)
        {
            ret = false;
            break;
        }
    }
    return ret;
}


uint8_t tuya_ble_check_sum(uint8_t *pbuf,uint16_t len)
{
    uint32_t i = 0,ck_sum = 0;

    for(i = 0; i<len ; i++)
        ck_sum += pbuf[i];
    return (uint8_t)ck_sum;
}

uint8_t tuya_ble_check_num(uint8_t *buf,uint8_t num)
{
    uint8_t i = 0;

    for(; i < buf[0]; i++) {
        if(buf[i+1] == num) {
            return 1;
        }
    }
    return 0;
}

void tuya_ble_hextoascii(uint8_t *hexbuf,uint8_t len,uint8_t *ascbuf)
{
    uint8_t i =0,j =0,temp = 0;

    for(i = 0; i<len; i++) {
        temp = (hexbuf[i]>>4)&0xf;
        if(temp <=9) {
            ascbuf[j] = temp + 0x30;
        }
        else {
            ascbuf[j] = temp + 87;
        }
        j++;
        temp = (hexbuf[i])&0xf;
        if(temp <=9) {
            ascbuf[j] = temp + 0x30;
        }
        else {
            ascbuf[j] = temp + 87;
        }
        j++;
    }
    ascbuf[j] = 0x0;
}

void tuya_ble_asciitohex(uint8_t *ascbuf,uint8_t *hexbuf)
{
    uint8_t i =0,j =0;

    while(ascbuf[i]) {
        j++;
        if((ascbuf[i] >= 0x30)&&(ascbuf[i] <= 0x39)) {
            hexbuf[j] = ((ascbuf[i] - 0x30)<<4);
        }
        else if((ascbuf[i] >= 65)&&(ascbuf[i] <= 70)) {
            hexbuf[j] = ((ascbuf[i] - 55)<<4);
        }
        else if((ascbuf[i] >= 97)&&(ascbuf[i] <= 102)) {
            hexbuf[j] = ((ascbuf[i] - 87)<<4);
        }
        i++;
        if((ascbuf[i] >= 0x30)&&(ascbuf[i] <= 0x39)) {
            hexbuf[j] |= (ascbuf[i] - 0x30);
        }
        else if((ascbuf[i] >= 65)&&(ascbuf[i] <= 70)) {
            hexbuf[j] |= (ascbuf[i] - 55);
        }
        else if((ascbuf[i] >= 97)&&(ascbuf[i] <= 102)) {
            hexbuf[j] |= (ascbuf[i] - 87);
        }
        i++;
    }
    hexbuf[0] = j;
}



uint8_t tuya_ble_char_2_ascii(uint8_t data)
{
    uint8_t ret = 0xff;

    if((data >= 48)&&(data <= 57)) {
        ret = data - 48;
    }
    else if((data >= 65)&&(data <= 70)) {
        ret = data - 55;
    }
    else if((data >= 97)&&(data <= 102)) {
        ret = data - 87;
    }
    return ret;
}

void tuya_ble_str_to_hex(uint8_t *str_buf,uint8_t str_len,uint8_t *hex_buf)
{
    uint8_t data_tmp = 0,i = 0,j = 0;

    for(j = 0 ; j < str_len ; j++) {
        data_tmp = tuya_ble_char_2_ascii(str_buf[j]);
        if(data_tmp != 0xff) {
            hex_buf[i] = (data_tmp << 4);
        }
        else {
            return;
        }
        j++;
        data_tmp = tuya_ble_char_2_ascii(str_buf[j]);
        if(data_tmp != 0xff) {
            hex_buf[i] += data_tmp;
        }
        else {
            return;
        }
        i++;
    }
}

void tuya_ble_swap(int16_t *a, int16_t *b)
{
    int16_t    temp;
    temp = *a;
    *a = *b;
    *b = temp;
}


int32_t tuya_ble_hex2int(uint8_t mhex)
{
    switch(mhex) {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
    case 'A':
        return 10;
    case 'b':
    case 'B':
        return 11;
    case 'c':
    case 'C':
        return 12;
    case 'd':
    case 'D':
        return 13;
    case 'e':
    case 'E':
        return 14;
    case 'f':
    case 'F':
        return 15;
    default:
        return -1;
    }
}
char tuya_ble_hexstr2int(uint8_t *hexstr,int len,uint8_t *sum)
{
    *sum = 0;
    int value;
    for(int i=0; i<len; i++) {
        value = tuya_ble_hex2int(hexstr[i]);
        if(value == -1)return 0;
        (*sum)=(*sum)<<4;
        (*sum)+=value;
    }
    return 1;
}
char tuya_ble_hexstr2hex(uint8_t *hexstr,int len,uint8_t *hex)
{
    for(uint8_t i=0; i<len; i+=2) {
        if(tuya_ble_hexstr2int(&hexstr[i],2,&hex[i/2]) == 0)
            return 0;
    }
    return 1;
}


static void swapX(const uint8_t *src, uint8_t *dst, int len)
{
    int i;
    for (i = 0; i < len; i++)
        dst[len - 1 - i] = src[i];
}

void tuya_ble_swap24(uint8_t dst[3], const uint8_t src[3])
{
    swapX(src, dst, 3);
}

void tuya_ble_swap32(uint8_t dst[4], const uint8_t src[4])
{
    swapX(src, dst, 4);
}

void tuya_ble_swap48(uint8_t dst[7], const uint8_t src[7])
{
    swapX(src, dst, 6);
}

void tuya_ble_swap56(uint8_t dst[7], const uint8_t src[7])
{
    swapX(src, dst, 7);
}

void tuya_ble_swap64(uint8_t dst[8], const uint8_t src[8])
{
    swapX(src, dst, 8);
}

void tuya_ble_swap128(uint8_t dst[16], const uint8_t src[16])
{
    swapX(src, dst, 16);
}



uint16_t tuya_ble_crc16_compute(uint8_t * p_data, uint16_t size, uint16_t * p_crc) {

    uint16_t poly[2] = {0, 0xa001};            //0x8005 <==> 0xa001
    uint16_t crc = 0xffff;

    int i, j;

    for(j = size; j > 0; j--) {
        unsigned char ds = *p_data++;

        for(i = 0; i < 8; i++) {
            crc = (crc >> 1) ^ poly[(crc ^ ds) & 1];
            ds = ds >> 1;
        }
    }

    return crc;
}


uint32_t tuya_ble_crc32_compute(uint8_t const * p_data, uint32_t size, uint32_t const * p_crc)
{
    uint32_t crc;
    crc = (p_crc == NULL) ? 0xFFFFFFFF : ~(*p_crc);
    for (uint32_t i = 0; i < size; i++)
    {
        crc = crc ^ p_data[i];
        for (uint32_t j = 8; j > 0; j--)
        {
            crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
        }
    }
    return ~crc;
}



/**@brief Function for checking if a pointer value is aligned to a 4 byte boundary.
 *
 * @param[in]   p   Pointer value to be checked.
 *
 * @return      TRUE if pointer is aligned to a 4 byte boundary, FALSE otherwise.
 */
bool tuya_ble_is_word_aligned_tuya(void const* p)
{
    return (((uintptr_t)p & 0x03) == 0);
}



void tuya_ble_device_id_20_to_16(uint8_t *in,uint8_t *out)
{
    uint8_t i,j;
    uint8_t temp[4];
    for(i=0; i<5; i++)
    {
        for(j=i*4; j<(i*4+4); j++)
        {
            if((in[j] >= 0x30)&&(in[j] <= 0x39))
            {
                temp[j-i*4] = in[j] - 0x30;
            }
            else if((in[j] >= 0x41)&&(in[j] <= 0x5A))
            {
                temp[j-i*4] = in[j] - 0x41 + 36;
            }
            else if((in[j] >= 0x61)&&(in[j] <= 0x7A))
            {
                temp[j-i*4] = in[j] - 0x61 + 10;
            }
            else
            {
                
            }
        }
        
        out[i*3] = temp[0]&0x3F;
        out[i*3] <<= 2;
        out[i*3] |= ((temp[1]>>4)&0x03);
        
        out[i*3+1] = temp[1]&0x0F;
        out[i*3+1] <<= 4;
        out[i*3+1] |= ((temp[2]>>2)&0x0F);
        
        out[i*3+2] = temp[2]&0x03;
        out[i*3+2] <<= 6;
        out[i*3+2] |= temp[3]&0x3F;
        
    }
    
    out[15] = 0xFF;
}

void tuya_ble_device_id_16_to_20(uint8_t *in,uint8_t *out)
{
    uint8_t i,j;
    uint8_t temp[4];
    for(i=0; i<5; i++)
    {
        j = i*3;   
        temp[j-i*3] = (in[j]>>2)&0x3F;
        temp[j-i*3+1] = in[j]&0x03;
        temp[j-i*3+1] <<= 4;
        temp[j-i*3+1] |= (in[j+1]>>4)&0x0F;
        temp[j-i*3+2] = (in[j+1]&0x0F)<<2;
        temp[j-i*3+2] |= ((in[j+2]&0xC0)>>6)&0x03;
        temp[j-i*3+3] = in[j+2]&0x3F;
        
        for(j=i*4; j<(i*4+4); j++)
        {
            if((temp[j-i*4] >= 0)&&(temp[j-i*4] <= 9))
            {
                out[j] = temp[j-i*4]+0x30;
            }
            else if((temp[j-i*4] >= 10)&&(temp[j-i*4] <= 35))
            {
                out[j] = temp[j-i*4] + 87;
            }
            else if((temp[j-i*4] >= 36)&&(temp[j-i*4] <= 61))
            {
                out[j] = temp[j-i*4] + 29;
            }
            else
            {
                
            }
        }
       
        
    }
    
}


