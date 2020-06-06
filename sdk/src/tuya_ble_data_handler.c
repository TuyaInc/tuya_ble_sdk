/**
 * \file tuya_ble_data_handler.c
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
#include "tuya_ble_type.h"
#include "tuya_ble_heap.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_main.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_unix_time.h"
#include "tuya_ble_log.h"
#include "tuya_ble_gatt_send_queue.h"


static uint32_t tuya_ble_firmware_version = 0;
static uint32_t tuya_ble_hardware_version = 0;

static uint32_t tuya_ble_mcu_firmware_version = 0;
static uint32_t tuya_ble_mcu_hardware_version = 0;


static  tuya_ble_r_air_recv_packet  air_recv_packet;

static frm_trsmitr_proc_s ty_trsmitr_proc;
static frm_trsmitr_proc_s ty_trsmitr_proc_send;

uint8_t tuya_ble_pair_rand[6] = {0};
uint8_t tuya_ble_pair_rand_valid = 0;

static uint32_t tuya_ble_receive_sn = 0;
static uint32_t tuya_ble_send_sn = 1;


tuya_ble_ota_status_t tuya_ble_ota_status;


static uint32_t get_ble_send_sn(void)
{
    uint32_t sn;
    tuya_ble_device_enter_critical();
    sn = tuya_ble_send_sn++;
    tuya_ble_device_exit_critical();
    return sn;
}

static void set_ble_receive_sn(uint32_t sn)
{
    tuya_ble_device_enter_critical();
    tuya_ble_receive_sn = sn;
    tuya_ble_device_exit_critical();
}

void tuya_ble_reset_ble_sn(void)
{
    tuya_ble_device_enter_critical();
    tuya_ble_receive_sn = 0;
    tuya_ble_send_sn = 1;
    tuya_ble_device_exit_critical();
}


void tuya_ble_set_device_version(uint32_t firmware_version,uint32_t hardware_version)
{
    tuya_ble_firmware_version = firmware_version;
    tuya_ble_hardware_version = hardware_version;
}


void tuya_ble_set_external_mcu_version(uint32_t firmware_version,uint32_t hardware_version)
{
    tuya_ble_device_enter_critical();
    tuya_ble_mcu_firmware_version = firmware_version;
    tuya_ble_mcu_hardware_version = hardware_version;
    tuya_ble_device_exit_critical();
}


void tuya_ble_ota_status_set(tuya_ble_ota_status_t status)
{
    tuya_ble_ota_status = status;
}


tuya_ble_ota_status_t tuya_ble_ota_status_get(void)
{
    return tuya_ble_ota_status;
}


void tuya_ble_pair_rand_clear(void)
{
    tuya_ble_device_enter_critical();
    memset(tuya_ble_pair_rand,0,sizeof(tuya_ble_pair_rand));
    tuya_ble_pair_rand_valid = 0;
    tuya_ble_device_exit_critical();
}


uint8_t tuya_ble_pair_rand_valid_get(void)
{
    return tuya_ble_pair_rand_valid;
}


static bool buffer_value_is_all_x(uint8_t *buffer,uint16_t len,uint8_t value)
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

void tuya_ble_air_recv_packet_free(void)
{
    if(air_recv_packet.recv_data)
    {
        tuya_ble_free(air_recv_packet.recv_data);
        air_recv_packet.recv_data = NULL;
        air_recv_packet.recv_len_max = 0;
        air_recv_packet.recv_len = 0;
    }
}

static uint32_t ble_data_unpack(uint8_t *buf,uint32_t len)
{
    static uint32_t offset = 0;
    mtp_ret ret;

    ret = trsmitr_recv_pkg_decode(&ty_trsmitr_proc, buf, len);
    if(MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret)
    {
        air_recv_packet.recv_len_max = 0;
        air_recv_packet.recv_len = 0;
        if(air_recv_packet.recv_data)
        {
            tuya_ble_free(air_recv_packet.recv_data);
            air_recv_packet.recv_data = NULL;
        }

        return 1;
    }

    if(FRM_PKG_FIRST == ty_trsmitr_proc.pkg_desc)
    {
        if(air_recv_packet.recv_data)
        {
            tuya_ble_free(air_recv_packet.recv_data);
            air_recv_packet.recv_data = NULL;
        }
        air_recv_packet.recv_len_max = get_trsmitr_frame_total_len(&ty_trsmitr_proc);
        if((air_recv_packet.recv_len_max>TUYA_BLE_AIR_FRAME_MAX)||(air_recv_packet.recv_len_max==0))
        {
            air_recv_packet.recv_len_max = 0;
            air_recv_packet.recv_len = 0;
            TUYA_BLE_LOG_ERROR("ble_data_unpack total size [%d ]error.",air_recv_packet.recv_len_max);
            return 2;
        }
        air_recv_packet.recv_len = 0;
        air_recv_packet.recv_data = tuya_ble_malloc(air_recv_packet.recv_len_max);
        if(air_recv_packet.recv_data==NULL)
        {
            TUYA_BLE_LOG_ERROR("ble_data_unpack malloc failed.");
            return 2;
        }
        memset(air_recv_packet.recv_data,0,air_recv_packet.recv_len_max);
        offset = 0;
    }
    if((offset+get_trsmitr_subpkg_len(&ty_trsmitr_proc))<=air_recv_packet.recv_len_max)
    {
        if(air_recv_packet.recv_data)
        {
            memcpy(air_recv_packet.recv_data+offset,get_trsmitr_subpkg(&ty_trsmitr_proc),get_trsmitr_subpkg_len(&ty_trsmitr_proc));
            offset += get_trsmitr_subpkg_len(&ty_trsmitr_proc);
            air_recv_packet.recv_len = offset;
        }
        else
        {
            TUYA_BLE_LOG_ERROR("ble_data_unpack error.");
            air_recv_packet.recv_len_max = 0;
            air_recv_packet.recv_len = 0;
            return 2;
        }
    }
    else
    {
        ret = MTP_INVALID_PARAM;
        TUYA_BLE_LOG_ERROR("ble_data_unpack[%d] error:MTP_INVALID_PARAM");
        tuya_ble_air_recv_packet_free();
    }

    if(ret == MTP_OK)
    {
        offset=0;
        TUYA_BLE_LOG_DEBUG("ble_data_unpack[%d]",air_recv_packet.recv_len);

        return 0;
    }
    else
    {
        return 2;
    }
}

static uint8_t ble_cmd_data_crc_check(uint8_t *input,uint16_t len)
{
    uint16_t data_len = 0;
    uint16_t crc16 = 0xFFFF;
    uint16_t crc16_cal = 0;

    data_len = (input[10]<<8)|input[11];

    if((13+data_len)>=TUYA_BLE_AIR_FRAME_MAX)
    {
        return 1;
    }

    crc16_cal = tuya_ble_crc16_compute(input,12+data_len, &crc16);

    TUYA_BLE_LOG_DEBUG("crc16_cal[0x%04x]",crc16_cal);
    crc16 = (input[12+data_len]<<8)|input[13+data_len];
    TUYA_BLE_LOG_DEBUG("crc16[0x%04x]",crc16);
    if(crc16==crc16_cal)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


void tuya_ble_commonData_rx_proc(uint8_t *buf,uint16_t len)
{
    uint8_t temp;
    uint32_t current_sn = 0;
    uint16_t current_cmd = 0;
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer=NULL;
    uint8_t current_encry_mode = 0;

    if(ble_data_unpack(buf,len))
    {
        return;      //
    }

    if(air_recv_packet.recv_len>TUYA_BLE_AIR_FRAME_MAX)
    {
        TUYA_BLE_LOG_ERROR("air_recv_packet.recv_len bigger than TUYA_BLE_AIR_FRAME_MAX.");
        tuya_ble_air_recv_packet_free();
        return;
    }

    if(ty_trsmitr_proc.version<2)  //协议主版本号低于2，不解析，返回。
    {
        TUYA_BLE_LOG_ERROR("ty_ble_rx_proc version not compatibility!");
        tuya_ble_air_recv_packet_free();
        return;
    }


    if(tuya_ble_current_para.sys_settings.bound_flag==1)//当前已绑定状态
    {
        if(ENCRYPTION_MODE_NONE==air_recv_packet.recv_data[0])
        {
            TUYA_BLE_LOG_ERROR("ty_ble_rx_proc data encryption mode error since bound_flag = 1.");
            tuya_ble_air_recv_packet_free();
            return;
        }
    }

    current_encry_mode = air_recv_packet.recv_data[0];

    TUYA_BLE_LOG_HEXDUMP_DEBUG("received encry data",(uint8_t*)air_recv_packet.recv_data,air_recv_packet.recv_len);//

    air_recv_packet.de_encrypt_buf = NULL;

    air_recv_packet.de_encrypt_buf = (uint8_t*)tuya_ble_malloc(air_recv_packet.recv_len);

    if(air_recv_packet.de_encrypt_buf==NULL)
    {
        TUYA_BLE_LOG_ERROR("air_recv_packet.de_encrypt_buf malloc failed.");
        tuya_ble_air_recv_packet_free();
        return;
    }
    else
    {
        air_recv_packet.decrypt_buf_len = 0;
        temp = tuya_ble_decryption((uint8_t *)air_recv_packet.recv_data,air_recv_packet.recv_len,&air_recv_packet.decrypt_buf_len,
                                   (uint8_t *)air_recv_packet.de_encrypt_buf,&tuya_ble_current_para,tuya_ble_pair_rand);
        tuya_ble_air_recv_packet_free();
    }


    if(temp != 0) //解密失败
    {
        TUYA_BLE_LOG_ERROR("ble receive data decryption error code = %d",temp);
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    }

    TUYA_BLE_LOG_HEXDUMP_DEBUG("decryped data",(uint8_t*)air_recv_packet.de_encrypt_buf,air_recv_packet.decrypt_buf_len);//解密数据
    //指令数据crc验证
    if(ble_cmd_data_crc_check((uint8_t *)air_recv_packet.de_encrypt_buf,air_recv_packet.decrypt_buf_len)!=0)
    {
        TUYA_BLE_LOG_ERROR("ble receive data crc check error!");
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    }

    //SN验证
    current_sn  = air_recv_packet.de_encrypt_buf[0]<<24;
    current_sn += air_recv_packet.de_encrypt_buf[1]<<16;
    current_sn += air_recv_packet.de_encrypt_buf[2]<<8;
    current_sn += air_recv_packet.de_encrypt_buf[3];

    if(current_sn<=tuya_ble_receive_sn)
    {
        TUYA_BLE_LOG_ERROR("ble receive SN error!");
        tuya_ble_gap_disconnect();//SN错误，断开蓝牙连接
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    }
    else
    {
        set_ble_receive_sn(current_sn);
    }

    current_cmd = ((air_recv_packet.de_encrypt_buf[8]<<8)|air_recv_packet.de_encrypt_buf[9]);


    if((BONDING_CONN != tuya_ble_connect_status_get())&&(FRM_QRY_DEV_INFO_REQ != current_cmd)&&(PAIR_REQ != current_cmd)
            &&(FRM_LOGIN_KEY_REQ != current_cmd)&&(FRM_FACTORY_TEST_CMD != current_cmd)&&(FRM_NET_CONFIG_INFO_REQ != current_cmd)&&(FRM_ANOMALY_UNBONDING_REQ != current_cmd))
    {   //没有绑定前，不响应其它命令
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        TUYA_BLE_LOG_ERROR("ble receive cmd error on current bond state!");
        return;
    }


    if(tuya_ble_ota_status_get()!=TUYA_BLE_OTA_STATUS_NONE)
    {   //OTA状态下，不处理其它事件
        if(!((current_cmd>=FRM_OTA_START_REQ)&&(current_cmd<=FRM_OTA_END_REQ)))
        {
            tuya_ble_free(air_recv_packet.de_encrypt_buf);
            TUYA_BLE_LOG_ERROR("ble receive cmd error on ota state!");
            return;
        }
    }

    ble_evt_buffer=(uint8_t*)tuya_ble_malloc(air_recv_packet.decrypt_buf_len+1);
    if(ble_evt_buffer==NULL)
    {
        TUYA_BLE_LOG_ERROR("ty_ble_rx_proc no mem.");
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    }
    else
    {
        memset(ble_evt_buffer,0,air_recv_packet.decrypt_buf_len+1);
    }
    ble_evt_buffer[0] = current_encry_mode;     //首字节拷贝加密方式，便于后续使用
    memcpy(ble_evt_buffer+1,(uint8_t *)air_recv_packet.de_encrypt_buf,air_recv_packet.decrypt_buf_len);
    evt.hdr.event = TUYA_BLE_EVT_BLE_CMD;
    evt.ble_cmd_data.cmd = current_cmd;
    evt.ble_cmd_data.p_data = ble_evt_buffer;
    evt.ble_cmd_data.data_len = air_recv_packet.decrypt_buf_len+1;
    TUYA_BLE_LOG_DEBUG("BLE EVENT SEND-CMD:0x%02x - LEN:0x%02x",current_cmd,air_recv_packet.decrypt_buf_len+1);

    if(tuya_ble_event_send(&evt)!=0)
    {
        TUYA_BLE_LOG_ERROR("ble event send fail!");
        tuya_ble_free(ble_evt_buffer);
    }

    tuya_ble_free(air_recv_packet.de_encrypt_buf);

}

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==3)

#if (TUYA_BLE_PROTOCOL_VERSION_LOW>=2)

static void tuya_ble_handle_dev_info_req(uint8_t*recv_data,uint16_t recv_len)
{
    uint8_t p_buf[90];
    uint8_t payload_len = 0;
    uint32_t ack_sn = 0;
    uint8_t encry_mode = 0;
    uint32_t version_temp_s,version_temp_h;

    memset(p_buf,0,sizeof(p_buf));

    ack_sn  = recv_data[1]<<24;
    ack_sn += recv_data[2]<<16;
    ack_sn += recv_data[3]<<8;
    ack_sn += recv_data[4];

    TUYA_BLE_LOG_DEBUG("get device infor-%d",tuya_ble_current_para.sys_settings.bound_flag);

    tuya_ble_rand_generator(tuya_ble_pair_rand,6);
    tuya_ble_pair_rand_valid = 1;

    if(TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
    {
        version_temp_s = tuya_ble_firmware_version;
        version_temp_h = tuya_ble_hardware_version;
        p_buf[4] = 0x00;
    }
    else
    {
        version_temp_s = tuya_ble_firmware_version>>8;
        version_temp_h = tuya_ble_hardware_version>>8;
        p_buf[4] = 0x05;
    }
    p_buf[0] = (version_temp_s>>8)&0xff;
    p_buf[1] = (version_temp_s&0xff);
    p_buf[2] = TUYA_BLE_PROTOCOL_VERSION_HIGN;
    p_buf[3] = TUYA_BLE_PROTOCOL_VERSION_LOW;
    if(TUYA_BLE_ADVANCED_ENCRYPTION_DEVICE==1)
    {
        p_buf[4] |= 0x02;
    }

    p_buf[5] = tuya_ble_current_para.sys_settings.bound_flag;
    memcpy(&p_buf[6],tuya_ble_pair_rand,6);
    p_buf[12] = (version_temp_h>>8)&0xff;
    p_buf[13] = (version_temp_h&0xff);

    tuya_ble_register_key_generate(&p_buf[14],&tuya_ble_current_para);

    p_buf[46] = (tuya_ble_firmware_version>>16)&0xff;
    p_buf[47] = (tuya_ble_firmware_version>>8)&0xff;
    p_buf[48] = (tuya_ble_firmware_version&0xff);
    p_buf[49] = (tuya_ble_hardware_version>>16)&0xff;
    p_buf[50] = (tuya_ble_hardware_version>>8)&0xff;
    p_buf[51] = (tuya_ble_hardware_version&0xff);

    p_buf[52] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY>>8;
    p_buf[53] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY; //communication ability

    p_buf[54] = 0x00;

    memcpy(&p_buf[55],tuya_ble_current_para.sys_settings.device_virtual_id,DEVICE_VIRTUAL_ID_LEN);

    p_buf[77] = (tuya_ble_mcu_firmware_version>>16)&0xff;
    p_buf[78] = (tuya_ble_mcu_firmware_version>>8)&0xff;
    p_buf[79] = (tuya_ble_mcu_firmware_version&0xff);
    p_buf[80] = (tuya_ble_mcu_hardware_version>>16)&0xff;
    p_buf[81] = (tuya_ble_mcu_hardware_version>>8)&0xff;
    p_buf[82] = (tuya_ble_mcu_hardware_version&0xff);

    p_buf[83] = TUYA_BLE_WIFI_DEVICE_REGISTER_MODE;

    payload_len = 84;

    encry_mode = recv_data[0];

    if(tuya_ble_commData_send(FRM_QRY_DEV_INFO_RESP,ack_sn,p_buf,payload_len,encry_mode)==2)
    {
        tuya_ble_pair_rand_clear();
    }
}

#else

static void tuya_ble_handle_dev_info_req(uint8_t*recv_data,uint16_t recv_len)
{
    uint8_t p_buf[90];
    uint8_t payload_len = 0;
    uint32_t ack_sn = 0;
    uint8_t encry_mode = 0;
    uint32_t version_temp_s,version_temp_h;

    ack_sn  = recv_data[1]<<24;
    ack_sn += recv_data[2]<<16;
    ack_sn += recv_data[3]<<8;
    ack_sn += recv_data[4];

    TUYA_BLE_LOG_DEBUG("get device infor-%d",tuya_ble_current_para.sys_settings.bound_flag);

    tuya_ble_rand_generator(tuya_ble_pair_rand,6);
    tuya_ble_pair_rand_valid = 1;

    if(TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
    {
        version_temp_s = tuya_ble_firmware_version;
        version_temp_h = tuya_ble_hardware_version;
        p_buf[4] = 0x00;
    }
    else
    {
        version_temp_s = tuya_ble_firmware_version>>8;
        version_temp_h = tuya_ble_hardware_version>>8;
        p_buf[4] = 0x05;
    }
    p_buf[0] = (version_temp_s>>8)&0xff;
    p_buf[1] = (version_temp_s&0xff);
    p_buf[2] = TUYA_BLE_PROTOCOL_VERSION_HIGN;
    p_buf[3] = TUYA_BLE_PROTOCOL_VERSION_LOW;
    if(TUYA_BLE_ADVANCED_ENCRYPTION_DEVICE==1)
    {
        p_buf[4] |= 0x02;
    }

    p_buf[5] = tuya_ble_current_para.sys_settings.bound_flag;
    memcpy(&p_buf[6],tuya_ble_pair_rand,6);
    p_buf[12] = (version_temp_h>>8)&0xff;
    p_buf[13] = (version_temp_h&0xff);
    memcpy(&p_buf[14],tuya_ble_current_para.auth_settings.auth_key,AUTH_KEY_LEN);

    p_buf[46] = (tuya_ble_firmware_version>>16)&0xff;
    p_buf[47] = (tuya_ble_firmware_version>>8)&0xff;
    p_buf[48] = (tuya_ble_firmware_version&0xff);
    p_buf[49] = (tuya_ble_hardware_version>>16)&0xff;
    p_buf[50] = (tuya_ble_hardware_version>>8)&0xff;
    p_buf[51] = (tuya_ble_hardware_version&0xff);

    p_buf[52] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY>>8;
    p_buf[53] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY; //communication ability

    p_buf[54] = 0x00;

    memcpy(&p_buf[55],tuya_ble_current_para.sys_settings.device_virtual_id,DEVICE_VIRTUAL_ID_LEN);

    p_buf[77] = (tuya_ble_mcu_firmware_version>>16)&0xff;
    p_buf[78] = (tuya_ble_mcu_firmware_version>>8)&0xff;
    p_buf[79] = (tuya_ble_mcu_firmware_version&0xff);
    p_buf[80] = (tuya_ble_mcu_hardware_version>>16)&0xff;
    p_buf[81] = (tuya_ble_mcu_hardware_version>>8)&0xff;
    p_buf[82] = (tuya_ble_mcu_hardware_version&0xff);

    payload_len = 83;

    encry_mode = recv_data[0];

    if(tuya_ble_commData_send(FRM_QRY_DEV_INFO_RESP,ack_sn,p_buf,payload_len,encry_mode)==2)
    {
        tuya_ble_pair_rand_clear();
    }
}

#endif


static void tuya_ble_storage_save_sys_settings_async_completed(void *p_param,tuya_ble_status_t result)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_SAVE_SYS_SETTINGS_DATA_CALLBACK;
    event.sys_settings_save_callback_data.type = 0;
    event.sys_settings_save_callback_data.status = result;

    if(tuya_ble_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_storage_save_sys_settings_async_completed event Send failed.");
    }
}


extern void tuya_ble_connect_monitor_timer_stop(void);
static void tuya_ble_handle_pair_req(uint8_t*recv_data,uint16_t recv_len)
{
    uint8_t p_buf[1];
    uint8_t encry_mode = 0;
    uint32_t ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

    ack_sn  = recv_data[1]<<24;
    ack_sn += recv_data[2]<<16;
    ack_sn += recv_data[3]<<8;
    ack_sn += recv_data[4];

    if(0 == memcmp(&recv_data[13],tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN))
    {
        tuya_ble_connect_monitor_timer_stop();

        if(1==tuya_ble_get_adv_connect_request_bit_status())
        {
            TUYA_BLE_LOG_INFO("ble adv data update,because the last broadcast data connection request flag was set!");
            tuya_ble_adv_change();
        }

        if(tuya_ble_current_para.sys_settings.bound_flag==1)
        {
            TUYA_BLE_LOG_INFO("PAIR_REQ already bound!");
            p_buf[0] = 2;

            tuya_ble_connect_status_set(BONDING_CONN);

            event.evt = TUYA_BLE_CB_EVT_CONNECTE_STATUS;
            event.connect_status = tuya_ble_connect_status_get();
            if(tuya_ble_cb_event_send(&event)!=0)
            {
                TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
            }
            else
            {
                TUYA_BLE_LOG_INFO("tuya ble send cb event succeed.");
            }

            encry_mode = ENCRYPTION_MODE_SESSION_KEY;

            tuya_ble_commData_send(PAIR_RESP,ack_sn,p_buf,1,encry_mode);


            tuya_ble_commData_send(FRM_GET_UNIX_TIME_CHAR_MS_REQ,0,NULL,0,encry_mode);
            TUYA_BLE_LOG_INFO("send FRM_GET_UNIX_TIME_CHAR_MS_REQ cmd to app.\n");

        }
        else
        {

#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
            memcpy(tuya_ble_current_para.sys_settings.login_key,recv_data+29,LOGIN_KEY_LEN);
            memcpy(tuya_ble_current_para.sys_settings.device_virtual_id,recv_data+29+LOGIN_KEY_LEN,DEVICE_VIRTUAL_ID_LEN);
            tuya_ble_current_para.sys_settings.bound_flag = 1;

#if (TUYA_BLE_DEVICE_AUTH_DATA_STORE)
            tuya_ble_storage_save_sys_settings_async(tuya_ble_storage_save_sys_settings_async_completed);
#else

            tuya_ble_adv_change();
            TUYA_BLE_LOG_INFO("PAIR_REQ ok-%d",tuya_ble_current_para.sys_settings.bound_flag);

            event.evt = TUYA_BLE_CB_EVT_UPDATE_LOGIN_KEY_VID;
            event.device_login_key_vid_data.login_key_len = LOGIN_KEY_LEN;
            event.device_login_key_vid_data.vid_len = DEVICE_VIRTUAL_ID_LEN;
            memcpy( event.device_login_key_vid_data.login_key,tuya_ble_current_para.sys_settings.login_key,LOGIN_KEY_LEN);
            memcpy( event.device_login_key_vid_data.vid,tuya_ble_current_para.sys_settings.device_virtual_id,DEVICE_VIRTUAL_ID_LEN);
            if(tuya_ble_cb_event_send(&event)!=0)
            {
                TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
            }
            else
            {
                TUYA_BLE_LOG_DEBUG("tuya ble send cb event succeed.");
            }
            p_buf[0] = 0x00;


            tuya_ble_connect_status_set(BONDING_CONN);

            event.evt = TUYA_BLE_CB_EVT_CONNECTE_STATUS;
            event.connect_status = tuya_ble_connect_status_get();
            if(tuya_ble_cb_event_send(&event)!=0)
            {
                TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
            }
            else
            {
                TUYA_BLE_LOG_INFO("tuya ble send cb event succeed.");
            }

            encry_mode = ENCRYPTION_MODE_SESSION_KEY;

            tuya_ble_commData_send(PAIR_RESP,ack_sn,p_buf,1,encry_mode);

            tuya_ble_commData_send(FRM_GET_UNIX_TIME_CHAR_MS_REQ,0,NULL,0,encry_mode);
            TUYA_BLE_LOG_INFO("send FRM_GET_UNIX_TIME_CHAR_MS_REQ cmd to app.\n");

#endif

#else
            tuya_ble_connect_status_set(UNBONDING_CONN);

            p_buf[0] = 0x00;

            event.evt = TUYA_BLE_CB_EVT_CONNECTE_STATUS;
            event.connect_status = tuya_ble_connect_status_get();
            if(tuya_ble_cb_event_send(&event)!=0)
            {
                TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
            }
            else
            {
                TUYA_BLE_LOG_INFO("tuya ble send cb event succeed.");
            }

            encry_mode = recv_data[0];

            tuya_ble_commData_send(PAIR_RESP,ack_sn,p_buf,1,encry_mode);

#endif

        }

    }
    else
    {
        TUYA_BLE_LOG_ERROR("PAIR_REQ device id not match!");  //ID not match ,and disconnected.
        p_buf[0] = 0x01;
        encry_mode = recv_data[0];
        tuya_ble_commData_send(PAIR_RESP,ack_sn,p_buf,1,encry_mode);
        tuya_ble_gap_disconnect();
    }

}

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==2)

static void tuya_ble_handle_dev_info_req(uint8_t*recv_data,uint16_t recv_len)
{
    uint8_t p_buf[70];
    uint8_t payload_len = 0;
    uint32_t ack_sn = 0;
    uint8_t encry_mode = 0;

    ack_sn  = recv_data[1]<<24;
    ack_sn += recv_data[2]<<16;
    ack_sn += recv_data[3]<<8;
    ack_sn += recv_data[4];

    TUYA_BLE_LOG_INFO("get device infor-%d",tuya_ble_current_para.sys_settings.bound_flag);

    tuya_ble_rand_generator(tuya_ble_pair_rand,6);
    tuya_ble_pair_rand_valid = 1;


    p_buf[0] = (tuya_ble_firmware_version>>8)&0xff;
    p_buf[1] = (tuya_ble_firmware_version&0xff);
    p_buf[2] = TUYA_BLE_PROTOCOL_VERSION_HIGN;
    p_buf[3] = TUYA_BLE_PROTOCOL_VERSION_LOW;
    if(TUYA_BLE_ADVANCED_ENCRYPTION_DEVICE==1)
    {
        p_buf[4] = 0x02;
    }
    else
    {
        p_buf[4] = 0x00;
    }
    p_buf[5] = tuya_ble_current_para.sys_settings.bound_flag;
    memcpy(&p_buf[6],tuya_ble_pair_rand,6);
    p_buf[12] = (tuya_ble_hardware_version>>8)&0xff;
    p_buf[13] = (tuya_ble_hardware_version&0xff);
    memcpy(&p_buf[14],tuya_ble_current_para.auth_settings.auth_key,AUTH_KEY_LEN);

    payload_len = 46;

    if(tuya_ble_current_para.sys_settings.bound_flag==1)
    {
        encry_mode = ENCRYPTION_MODE_KEY_4;
    }
    else
    {
        encry_mode = ENCRYPTION_MODE_KEY_1;
    }

    tuya_ble_commData_send(FRM_QRY_DEV_INFO_RESP,ack_sn,p_buf,payload_len,encry_mode);
}


static void tuya_ble_handle_pair_req(uint8_t*recv_data,uint16_t recv_len)
{
    uint8_t p_buf[1];
    uint8_t encry_mode = 0;
    uint32_t ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

    ack_sn  = recv_data[1]<<24;
    ack_sn += recv_data[2]<<16;
    ack_sn += recv_data[3]<<8;
    ack_sn += recv_data[4];

    if(0 == memcmp(&recv_data[13],tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN))
    {
        tuya_ble_connect_monitor_timer_stop();

        if(1==tuya_ble_get_adv_connect_request_bit_status())
        {
            TUYA_BLE_LOG_INFO("ble adv data update,because the last broadcast data connection request flag was set!");
            tuya_ble_adv_change();
        }

        if(tuya_ble_current_para.sys_settings.bound_flag==1)
        {
            TUYA_BLE_LOG_ERROR("PAIR_REQ already bound!");
            p_buf[0] = 2;
        }
        else
        {

#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)

            memcpy(tuya_ble_current_para.sys_settings.login_key,recv_data+29,LOGIN_KEY_LEN);
            tuya_ble_current_para.sys_settings.bound_flag = 1;
            tuya_ble_storage_save_sys_settings();
            tuya_ble_adv_change();
            TUYA_BLE_LOG_INFO("PAIR_REQ ok-%d",tuya_ble_current_para.sys_settings.bound_flag);
#else
            tuya_ble_connect_status_set(UNBONDING_CONN);
#endif
            p_buf[0] = 0x00;

        }

        if(tuya_ble_current_para.sys_settings.bound_flag==1)
        {
            tuya_ble_connect_status_set(BONDING_CONN);
        }

        event.evt = TUYA_BLE_CB_EVT_CONNECTE_STATUS;
        event.connect_status = tuya_ble_connect_status_get();
        if(tuya_ble_cb_event_send(&event)!=0)
        {
            TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
        }
        else
        {
            TUYA_BLE_LOG_INFO("tuya ble send cb event succeed.");
        }


    }
    else
    {
        TUYA_BLE_LOG_ERROR("PAIR_REQ device id not match!");  //ID not match ,and disconnected.
        p_buf[0] = 0x01;

    }


    if(p_buf[0]==0)
    {
#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
#else
        encry_mode = recv_data[0];
#endif
    }
    else if(p_buf[0]==1)
    {
        encry_mode = recv_data[0];
    }
    else if(p_buf[0]==2)
    {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    }
    else
    {

    }

    tuya_ble_commData_send(PAIR_RESP,ack_sn,p_buf,1,encry_mode);

    if(encry_mode == ENCRYPTION_MODE_SESSION_KEY)
    {
        tuya_ble_commData_send(FRM_GET_UNIX_TIME_CHAR_MS_REQ,0,NULL,0,encry_mode);
        TUYA_BLE_LOG_INFO("send FRM_GET_UNIX_TIME_CHAR_MS_REQ cmd to app.");
    }

    if(p_buf[0]==1)
    {
        tuya_ble_gap_disconnect();
    }

}

#endif

static void tuya_ble_handle_net_config_info_req(uint8_t*recv_data,uint16_t recv_len)
{
    uint8_t p_buf[1];
    uint8_t encry_mode = 0;
    uint32_t ack_sn = 0;
    tuya_ble_cb_evt_param_t event;
    uint16_t data_len;


    ack_sn  = recv_data[1]<<24;
    ack_sn += recv_data[2]<<16;
    ack_sn += recv_data[3]<<8;
    ack_sn += recv_data[4];
    data_len = (recv_data[11]<<8) + recv_data[12];
    encry_mode = recv_data[0];

    event.evt = TUYA_BLE_CB_EVT_NETWORK_INFO;
    uint8_t *ble_cb_evt_buffer=(uint8_t*)tuya_ble_malloc(data_len);
    if(ble_cb_evt_buffer==NULL)
    {
        p_buf[0]=1;
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        tuya_ble_commData_send(FRM_NET_CONFIG_INFO_RESP,ack_sn,p_buf,1,encry_mode);
        return;
    }
    else
    {
        memset(ble_cb_evt_buffer,0,data_len);
        memcpy(ble_cb_evt_buffer,&recv_data[13],data_len);
    }
    event.network_data.data_len = data_len;
    event.network_data.p_data = ble_cb_evt_buffer;

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
        p_buf[0]=1;
        tuya_ble_commData_send(FRM_NET_CONFIG_INFO_RESP,ack_sn,p_buf,1,encry_mode);
        return;
    }
    else
    {
        p_buf[0]=0;
    }

    tuya_ble_commData_send(FRM_NET_CONFIG_INFO_RESP,ack_sn,p_buf,1,encry_mode);


}


static void tuya_ble_handle_ble_passthrough_data_req(uint8_t*recv_data,uint16_t recv_len)
{
    tuya_ble_cb_evt_param_t event;
    uint16_t data_len;

    data_len = (recv_data[11]<<8) + recv_data[12];

    event.evt = TUYA_BLE_CB_EVT_DATA_PASSTHROUGH;

    uint8_t *ble_cb_evt_buffer=(uint8_t*)tuya_ble_malloc(data_len);
    if(ble_cb_evt_buffer==NULL)
    {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        return;
    }
    else
    {
        memcpy(ble_cb_evt_buffer,&recv_data[13],data_len);
    }
    event.ble_passthrough_data.data_len = data_len;
    event.ble_passthrough_data.p_data = ble_cb_evt_buffer;

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
    }
    else
    {

    }

}

#include "tuya_ble_app_production_test.h"
static void tuya_ble_handle_ble_factory_test_req(uint8_t*recv_data,uint16_t recv_len)
{
    uint16_t data_len;
    uint32_t ack_sn;
    uint8_t sum;
    uint8_t encry_mode;

    data_len = (recv_data[11]<<8) + recv_data[12];

    if(data_len<7)
    {
        return;
    }

    ack_sn  = recv_data[1]<<24;
    ack_sn += recv_data[2]<<16;
    ack_sn += recv_data[3]<<8;
    ack_sn += recv_data[4];

    encry_mode = recv_data[0];

    if((recv_data[13]==0x66)&&(recv_data[14]==0xAA))
    {
        sum = tuya_ble_check_sum(&recv_data[13],data_len-1);
        if(sum==recv_data[13+data_len-1])
        {
            tuya_ble_app_production_test_process(1,&recv_data[13],data_len);
        }
    }

}


#include "tuya_ble_app_uart_common_handler.h"
static void tuya_ble_handle_ota_req(uint16_t cmd,uint8_t*recv_data,uint32_t recv_len)
{
    tuya_ble_cb_evt_param_t event;
    uint16_t data_len;
    tuya_ble_ota_data_type_t cmd_type;

    data_len = (recv_data[11]<<8) + recv_data[12];

    if(data_len==0)
    {
        return;
    }

    if(recv_data[13]==1)  //extern mcu ota
    {
        tuya_ble_uart_common_mcu_ota_data_from_ble_handler(cmd,&recv_data[14],data_len-1);
    }
    else if(recv_data[13]==0)
    {
        event.evt = TUYA_BLE_CB_EVT_OTA_DATA;

        uint8_t *ble_cb_evt_buffer=(uint8_t*)tuya_ble_malloc(data_len);
        if(ble_cb_evt_buffer==NULL)
        {
            TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
            return;
        }
        else
        {
            memcpy(ble_cb_evt_buffer,&recv_data[13],data_len);
        }

        switch (cmd)
        {
        case FRM_OTA_START_REQ:
            cmd_type = TUYA_BLE_OTA_REQ;
            break;
        case FRM_OTA_FILE_INFOR_REQ:
            cmd_type = TUYA_BLE_OTA_FILE_INFO;
            break;
        case FRM_OTA_FILE_OFFSET_REQ:
            cmd_type = TUYA_BLE_OTA_FILE_OFFSET_REQ;
            break;
        case FRM_OTA_DATA_REQ:
            cmd_type = TUYA_BLE_OTA_DATA;
            break;
        case FRM_OTA_END_REQ:
            cmd_type = TUYA_BLE_OTA_END;
            break;
        default:
            cmd_type = TUYA_BLE_OTA_UNKONWN;
            break;
        }

        event.ota_data.type = cmd_type;
        event.ota_data.data_len = data_len;
        event.ota_data.p_data = ble_cb_evt_buffer;

        if(tuya_ble_cb_event_send(&event)!=0)
        {
            tuya_ble_free(ble_cb_evt_buffer);
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_ota_req-tuya ble send cb event failed.");
        }
        else
        {

        }
    }
    else
    {

    }
}

static char current_timems_string[14] = "000000000000";

static void tuya_ble_handle_unix_time_char_ms_resp(uint8_t*recv_data,uint16_t recv_len)
{
    int16_t zone_temp = 0;
    uint64_t time_stamp_ms;
    uint32_t time_stamp;
    tuya_ble_cb_evt_param_t event;

    memset(&event,0,sizeof(tuya_ble_cb_evt_param_t));

    if(recv_len<30)
    {
        TUYA_BLE_LOG_ERROR("received unix time char cmd data length error!");
        return;
    }

    if(!buffer_value_is_all_x(&recv_data[13],13,0))
    {
        memcpy(current_timems_string,&recv_data[13],13);
        zone_temp  = (int16_t)((recv_data[26]<<8)|recv_data[27]);
        time_stamp_ms = atoll(current_timems_string);
        TUYA_BLE_LOG_INFO("received unix time_zone = %d\n",zone_temp);
        time_stamp = time_stamp_ms/1000;
        if(time_stamp_ms%1000>=500)
        {
            time_stamp += 1;
        }

        tuya_ble_rtc_set_timestamp(time_stamp,zone_temp);

        event.evt = TUYA_BLE_CB_EVT_TIME_STAMP;

        memcpy(event.timestamp_data.timestamp_string,current_timems_string,13);
        event.timestamp_data.time_zone = zone_temp;
        if(tuya_ble_cb_event_send(&event)!=0)
        {
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_unix_time_char_ms_resp-tuya ble send cb event failed.");
        }

    }

}


static void tuya_ble_handle_unix_time_date_resp(uint8_t*recv_data,uint16_t recv_len)
{
    int16_t zone_temp = 0;
    uint32_t time_stamp;
    tuya_ble_cb_evt_param_t event;
    tuya_ble_time_struct_data_t time_temp;

    if(recv_len<24)
    {
        TUYA_BLE_LOG_ERROR("received unix time date cmd data length error!");
        return;
    }

    memset(&event,0,sizeof(tuya_ble_cb_evt_param_t));

    if(!buffer_value_is_all_x(&recv_data[13],7,0))
    {
        time_temp.nYear = 2000+recv_data[13];
        time_temp.nMonth = recv_data[14];
        time_temp.nDay = recv_data[15];
        time_temp.nHour = recv_data[16];
        time_temp.nMin = recv_data[17];
        time_temp.nSec = recv_data[18];
        time_temp.DayIndex = recv_data[19];

        time_stamp = tuya_ble_mytime_2_utc_sec(&time_temp,false);

        zone_temp  = (int16_t)((recv_data[20]<<8)|recv_data[21]);

        TUYA_BLE_LOG_INFO("received unix time_zone = %d",zone_temp);

        tuya_ble_rtc_set_timestamp(time_stamp,zone_temp);

        event.evt = TUYA_BLE_CB_EVT_TIME_NORMAL;
        event.time_normal_data.nYear = recv_data[13];
        event.time_normal_data.nMonth = recv_data[14];
        event.time_normal_data.nDay = recv_data[15];
        event.time_normal_data.nHour = recv_data[16];
        event.time_normal_data.nMin = recv_data[17];
        event.time_normal_data.nSec = recv_data[18];
        event.time_normal_data.DayIndex = recv_data[19];
        event.time_normal_data.time_zone = zone_temp;

        if(tuya_ble_cb_event_send(&event)!=0)
        {
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_unix_time_date_resp-tuya ble send cb event failed.");
        }

    }

}

static void tuya_ble_handle_dp_write_req(uint8_t*recv_data,uint16_t recv_len)
{
    mtp_ret ret;
    klv_node_s *list = NULL;
    uint8_t p_buf[1];
    uint16_t data_len = 0;
    uint32_t ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

    ack_sn  = recv_data[1]<<24;
    ack_sn += recv_data[2]<<16;
    ack_sn += recv_data[3]<<8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11]<<8)|recv_data[12];

    if((data_len==0)||(data_len>TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN))
    {
        TUYA_BLE_LOG_ERROR("cmd dp write receive data len == %d",data_len);
        p_buf[0] = 0x01;
        tuya_ble_commData_send(FRM_CMD_RESP,ack_sn,p_buf,1,ENCRYPTION_MODE_SESSION_KEY);
        return;
    }
    TUYA_BLE_LOG_HEXDUMP_DEBUG("cmd_dp_write data : ",recv_data+13,data_len);
    ret = data_2_klvlist(&recv_data[13],data_len,&list,0);
    if(MTP_OK != ret)
    {
        TUYA_BLE_LOG_ERROR("cmd rx fail-%d",ret);
        p_buf[0] = 0x01;
        tuya_ble_commData_send(FRM_CMD_RESP,ack_sn,p_buf,1,ENCRYPTION_MODE_SESSION_KEY);
        return;
    }

    free_klv_list(list);
    p_buf[0] = 0x00;

    tuya_ble_commData_send(FRM_CMD_RESP,ack_sn,p_buf,1,ENCRYPTION_MODE_SESSION_KEY);

    event.evt = TUYA_BLE_CB_EVT_DP_WRITE;

    uint8_t *ble_cb_evt_buffer=(uint8_t*)tuya_ble_malloc(data_len);
    if(ble_cb_evt_buffer==NULL)
    {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        return;
    }
    else
    {
        memcpy(ble_cb_evt_buffer,&recv_data[13],data_len);
    }
    event.dp_write_data.p_data = ble_cb_evt_buffer;
    event.dp_write_data.data_len = data_len;

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_write_req-tuya ble send cb event failed.");
    }
    else
    {

    }

}

static void tuya_ble_handle_dp_query_req(uint8_t*recv_data,uint16_t recv_len)
{
    uint8_t p_buf[1];
    uint16_t dp_num = 0;
    uint32_t ack_sn = 0;
    tuya_ble_cb_evt_param_t event;
    uint8_t *ble_cb_evt_buffer = NULL;

    ack_sn  = recv_data[1]<<24;
    ack_sn += recv_data[2]<<16;
    ack_sn += recv_data[3]<<8;
    ack_sn += recv_data[4];

    dp_num = (recv_data[11]<<8)|recv_data[12];

    p_buf[0] = 0x00;

    tuya_ble_commData_send(FRM_STATE_QUERY_RESP,ack_sn,p_buf,1,ENCRYPTION_MODE_SESSION_KEY);

    event.evt = TUYA_BLE_CB_EVT_DP_QUERY;

    if(dp_num>0)
    {
        ble_cb_evt_buffer=(uint8_t*)tuya_ble_malloc(dp_num);
        if(ble_cb_evt_buffer==NULL)
        {
            TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
            return;
        }
        else
        {
            memcpy(ble_cb_evt_buffer,&recv_data[13],dp_num);
        }
    }
    event.dp_query_data.p_data = ble_cb_evt_buffer;
    event.dp_query_data.data_len = dp_num;

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_query_req-tuya ble send cb event failed.");
        if(dp_num>0)
        {
            tuya_ble_free(ble_cb_evt_buffer);
        }
    }
    else
    {

    }
}

static void tuya_ble_device_unbond_completed(void *p_param,tuya_ble_status_t result)
{
    if(result!=TUYA_BLE_SUCCESS)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_device_unbond save sys settings data failed,reset....");
        tuya_ble_device_reset();  //must be reset!
    }
}

void tuya_ble_device_unbond(void)
{
    tuya_ble_storage_save_sys_settings_async(tuya_ble_device_unbond_completed);
}


static void tuya_ble_handle_unbond_req(uint8_t*recv_data,uint16_t recv_len)
{
    uint8_t p_buf[1];
    uint8_t encry_mode = 0;
    uint32_t ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

    event.evt = TUYA_BLE_CB_EVT_UNBOUND;
    event.unbound_data.data = 0;

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_unbond_req-tuya ble send cb event (unbound req) failed.");
    }
    else
    {

    }
#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
    ack_sn  = recv_data[1]<<24;
    ack_sn += recv_data[2]<<16;
    ack_sn += recv_data[3]<<8;
    ack_sn += recv_data[4];

    encry_mode = ENCRYPTION_MODE_SESSION_KEY;

    p_buf[0] = 0;

    tuya_ble_commData_send(FRM_UNBONDING_RESP,ack_sn,p_buf,1,encry_mode);

    tuya_ble_gap_disconnect();

    memset(tuya_ble_current_para.sys_settings.login_key,0,LOGIN_KEY_LEN);
    tuya_ble_current_para.sys_settings.bound_flag= 0;
    tuya_ble_adv_change();
    tuya_ble_connect_status_set(UNBONDING_UNCONN);
    TUYA_BLE_LOG_INFO("tuya_ble_device_unbond current bound flag = %d",tuya_ble_current_para.sys_settings.bound_flag);

    event.evt = TUYA_BLE_CB_EVT_CONNECTE_STATUS;
    event.connect_status = tuya_ble_connect_status_get();

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_unbond_req-tuya ble send cb event (connect status update) failed.");
    }
    else
    {

    }

    tuya_ble_device_unbond();

#endif
}


static void tuya_ble_handle_anomaly_unbond_req(uint8_t*recv_data,uint16_t recv_len)
{
    uint8_t p_buf[1];
    uint8_t encry_mode = 0;
    uint32_t ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

    event.evt = TUYA_BLE_CB_EVT_ANOMALY_UNBOUND;
    event.anomaly_unbound_data.data = 0;

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_anomaly_unbond_req-tuya ble send cb event (unbound req) failed.");
    }
    else
    {

    }

#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)

    ack_sn  = recv_data[1]<<24;
    ack_sn += recv_data[2]<<16;
    ack_sn += recv_data[3]<<8;
    ack_sn += recv_data[4];

    encry_mode = ENCRYPTION_MODE_KEY_1;

    p_buf[0] = 0;

    tuya_ble_commData_send(FRM_ANOMALY_UNBONDING_RESP,ack_sn,p_buf,1,encry_mode);
    
    tuya_ble_gap_disconnect();

    memset(tuya_ble_current_para.sys_settings.login_key,0,LOGIN_KEY_LEN);
    tuya_ble_current_para.sys_settings.bound_flag= 0;
    tuya_ble_adv_change();
    tuya_ble_connect_status_set(UNBONDING_UNCONN);
    TUYA_BLE_LOG_INFO("tuya_ble_handle_anomaly_unbond_req - current bound flag = %d",tuya_ble_current_para.sys_settings.bound_flag);

    event.evt = TUYA_BLE_CB_EVT_CONNECTE_STATUS;
    event.connect_status = tuya_ble_connect_status_get();

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_anomaly_unbond_req-tuya ble send cb event (connect status update) failed.");
    }
    else
    {

    }

    tuya_ble_device_unbond();
    
#endif
}


static void tuya_ble_handle_device_reset_req(uint8_t*recv_data,uint16_t recv_len)
{
    uint8_t p_buf[1];
    uint8_t encry_mode = 0;
    uint32_t ack_sn = 0;
    tuya_ble_cb_evt_param_t event;

    event.evt = TUYA_BLE_CB_EVT_DEVICE_RESET;
    event.device_reset_data.data = 0;

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_device_reset_req-tuya ble send cb event device reset req failed.");
    }
    else
    {

    }

#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE)

    ack_sn  = recv_data[1]<<24;
    ack_sn += recv_data[2]<<16;
    ack_sn += recv_data[3]<<8;
    ack_sn += recv_data[4];

    encry_mode = ENCRYPTION_MODE_SESSION_KEY;

    p_buf[0] = 0;

    tuya_ble_commData_send(FRM_DEVICE_RESET_RESP,ack_sn,p_buf,1,encry_mode);
    memset(tuya_ble_current_para.sys_settings.device_virtual_id,0,DEVICE_VIRTUAL_ID_LEN);
    
    tuya_ble_gap_disconnect();

    memset(tuya_ble_current_para.sys_settings.login_key,0,LOGIN_KEY_LEN);
    tuya_ble_current_para.sys_settings.bound_flag= 0;
    tuya_ble_adv_change();
    tuya_ble_connect_status_set(UNBONDING_UNCONN);
    TUYA_BLE_LOG_INFO("tuya_ble_device_unbond current bound flag = %d",tuya_ble_current_para.sys_settings.bound_flag);


    event.evt = TUYA_BLE_CB_EVT_CONNECTE_STATUS;
    event.connect_status = tuya_ble_connect_status_get();

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_device_reset_req-tuya ble send cb event (connect status update) failed.");
    }
    else
    {

    }
    
    tuya_ble_device_unbond();
#endif
}

static void tuya_ble_handle_dp_data_report_res(uint8_t*recv_data,uint16_t recv_len)
{
    tuya_ble_cb_evt_param_t event;

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_REPORT_RESPONSE;
    event.dp_response_data.status = recv_data[13];

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_report_res-tuya ble send cb event failed.");
    }
    else
    {

    }
}

static void tuya_ble_handle_dp_data_with_time_report_res(uint8_t*recv_data,uint16_t recv_len)
{
    tuya_ble_cb_evt_param_t event;

    event.evt = TUYA_BLE_CB_EVT_DP_DATA_WTTH_TIME_REPORT_RESPONSE;
    event.dp_response_data.status = recv_data[13];

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_report_res-tuya ble send cb event failed.");
    }
    else
    {

    }
}

/**
recv_data[0]为加密方式，1/2/3/4为SN
**/

void tuya_ble_evt_process(uint16_t cmd,uint8_t*recv_data,uint32_t recv_len)
{
    switch(cmd)
    {
    case FRM_CMD_SEND:
        tuya_ble_handle_dp_write_req(recv_data,recv_len);
        break;
    case FRM_STATE_QUERY:
        tuya_ble_handle_dp_query_req(recv_data,recv_len);
        break;
    case FRM_QRY_DEV_INFO_REQ:
        tuya_ble_handle_dev_info_req(recv_data,recv_len);
        break;
    case PAIR_REQ:
        tuya_ble_handle_pair_req(recv_data,recv_len);
        break;
    case FRM_NET_CONFIG_INFO_REQ:
        tuya_ble_handle_net_config_info_req(recv_data,recv_len);
        break;
    case FRM_DATA_PASSTHROUGH_REQ:
        tuya_ble_handle_ble_passthrough_data_req(recv_data,recv_len);
        break;
    case FRM_OTA_START_REQ:
    case FRM_OTA_FILE_INFOR_REQ:
    case FRM_OTA_FILE_OFFSET_REQ:
    case FRM_OTA_DATA_REQ:
    case FRM_OTA_END_REQ:
        TUYA_BLE_LOG_INFO("RECEIVED OTA CMD:0x%02x DATA LEN:0x%02x",cmd,recv_len);
        tuya_ble_handle_ota_req(cmd,recv_data,recv_len);
        break;
    case FRM_GET_UNIX_TIME_CHAR_MS_RESP:
        tuya_ble_handle_unix_time_char_ms_resp(recv_data,recv_len);
        break;
    case FRM_GET_UNIX_TIME_CHAR_DATE_RESP:
        tuya_ble_handle_unix_time_date_resp(recv_data,recv_len);
        break;
    case FRM_UNBONDING_REQ:
        TUYA_BLE_LOG_INFO("RECEIVED FRM_UNBONDING_REQ");
        tuya_ble_handle_unbond_req(recv_data,recv_len);
        break;
    case FRM_ANOMALY_UNBONDING_REQ:
        TUYA_BLE_LOG_INFO("RECEIVED FRM_ANOMALY_UNBONDING_REQ");
        tuya_ble_handle_anomaly_unbond_req(recv_data,recv_len);
        break;
    case FRM_DEVICE_RESET:
        TUYA_BLE_LOG_INFO("RECEIVED FRM_DEVICE_RESET_REQ");
        tuya_ble_handle_device_reset_req(recv_data,recv_len);
        break;
    case FRM_STAT_REPORT_RESP:
        tuya_ble_handle_dp_data_report_res(recv_data,recv_len);
        break;
    case FRM_STAT_WITH_TIME_REPORT_RESP:
        tuya_ble_handle_dp_data_with_time_report_res(recv_data,recv_len);
        break;
    case FRM_FACTORY_TEST_CMD:
        tuya_ble_handle_ble_factory_test_req(recv_data,recv_len);
        break;
    default:
        TUYA_BLE_LOG_WARNING("RECEIVED UNKNOWN BLE EVT CMD-0x%04x",cmd);
        break;
    }
}

uint8_t tuya_ble_commData_send(uint16_t cmd,uint32_t ack_sn,uint8_t *data,uint16_t len,uint8_t encryption_mode)
{
    mtp_ret ret;
    uint8_t send_len = 0;
    uint8_t p_buf[20];
    uint32_t err=0;
    int8_t retries_cnt = 0;
    uint8_t iv[16];
    uint16_t rand_value = 0,i=0;
    uint16_t crc16 = 0;
    uint16_t en_len  = 0;
    uint32_t out_len = 0;
    uint32_t temp_len = 0;
    uint32_t package_number = 0;
    tuya_ble_r_air_send_packet  air_send_packet;

    memset(&air_send_packet,0,sizeof(air_send_packet));

    tuya_ble_connect_status_t currnet_connect_status = tuya_ble_connect_status_get();

    if((currnet_connect_status == BONDING_UNCONN)||(currnet_connect_status== UNBONDING_UNCONN))
    {
        TUYA_BLE_LOG_ERROR("tuya ble commData_send failed,because ble not in connect status.");
        return 2;
    }

    if((encryption_mode>=ENCRYPTION_MODE_MAX)||(len>(TUYA_BLE_AIR_FRAME_MAX-29)))
    {
        return 1;
    }

    //生成随机IV
    if(encryption_mode != ENCRYPTION_MODE_NONE)
    {
        for(i=0; i<16; i+=2)
        {
            rand_value = rand();
            iv[i+0] = rand_value>>8;
            iv[i+1] = rand_value;
        }
        en_len = 17;
    }
    else
    {
        en_len = 1;
        memset(iv,0,sizeof(iv));
    }

    air_send_packet.send_len = 14+len;

    if(air_send_packet.send_len%16==0)
    {
        temp_len = 0;
    }
    else
    {
        temp_len = 16 - air_send_packet.send_len%16;
    }

    temp_len += air_send_packet.send_len;

    if(temp_len>(TUYA_BLE_AIR_FRAME_MAX-en_len))
    {
        TUYA_BLE_LOG_ERROR("The length of the send to ble exceeds the maximum length.");
        air_send_packet.send_len = 0;
        return 1; //加密后数据加上加密头超过AIR_FRAME_MAX
    }

    air_send_packet.send_data = NULL;

    air_send_packet.send_data = (uint8_t *)tuya_ble_malloc(temp_len); //must temp_len

    if(air_send_packet.send_data==NULL)
    {
        TUYA_BLE_LOG_ERROR("air_send_packet.send_data malloc failed return 3.");
        air_send_packet.send_len = 0;
        return 3;
    }
    else
    {
        memset(air_send_packet.send_data,0,temp_len);
    }


    uint32_t send_sn = get_ble_send_sn();
    //利用send_data buffer缓存明文指令数据
    air_send_packet.send_data[0] = send_sn>>24;
    air_send_packet.send_data[1] = send_sn>>16;
    air_send_packet.send_data[2] = send_sn>>8;
    air_send_packet.send_data[3] = send_sn;

    air_send_packet.send_data[4] = ack_sn>>24;
    air_send_packet.send_data[5] = ack_sn>>16;
    air_send_packet.send_data[6] = ack_sn>>8;
    air_send_packet.send_data[7] = ack_sn;

    air_send_packet.send_data[8] = cmd>>8;
    air_send_packet.send_data[9] = cmd;

    air_send_packet.send_data[10] = len>>8;
    air_send_packet.send_data[11] = len;

    memcpy(&air_send_packet.send_data[12],data,len);

    crc16 = tuya_ble_crc16_compute(air_send_packet.send_data,12+len, NULL);

    air_send_packet.send_data[12+len] = crc16>>8;
    air_send_packet.send_data[13+len] = crc16;


    TUYA_BLE_LOG_HEXDUMP_DEBUG("ble_commData_send plain data",(uint8_t*)air_send_packet.send_data,air_send_packet.send_len);//

    air_send_packet.encrypt_data_buf = NULL;

    air_send_packet.encrypt_data_buf = (uint8_t *)tuya_ble_malloc(temp_len+en_len);

    if(air_send_packet.encrypt_data_buf==NULL)
    {
        TUYA_BLE_LOG_ERROR("air_send_packet.encrypt_data_buf malloc failed.");
        tuya_ble_free(air_send_packet.send_data);
        return 3;
    }
    else
    {
        air_send_packet.encrypt_data_buf_len = 0;
        memset(air_send_packet.encrypt_data_buf,0,temp_len+en_len);
    }


    air_send_packet.encrypt_data_buf[0] = encryption_mode;

    if(encryption_mode != ENCRYPTION_MODE_NONE)
    {
        memcpy(&air_send_packet.encrypt_data_buf[1],iv,16);
    }
    else
    {

    }

    if(tuya_ble_encryption(encryption_mode,iv,(uint8_t *)air_send_packet.send_data,air_send_packet.send_len,&out_len,
                           (uint8_t *)(air_send_packet.encrypt_data_buf+en_len),&tuya_ble_current_para,tuya_ble_pair_rand)==0)
    {
        if(out_len!=temp_len)
        {
            TUYA_BLE_LOG_ERROR("ble_commData_send encryed error.");
            tuya_ble_free(air_send_packet.send_data);
            tuya_ble_free(air_send_packet.encrypt_data_buf);
            return 1;
        }

        air_send_packet.encrypt_data_buf_len = en_len + out_len;

        TUYA_BLE_LOG_HEXDUMP_DEBUG("ble_commData_send encryped data",(uint8_t*)air_send_packet.encrypt_data_buf,air_send_packet.encrypt_data_buf_len);//
    }
    else
    {
        TUYA_BLE_LOG_ERROR("ble_commData_send encryed fail.");
        tuya_ble_free(air_send_packet.send_data);
        tuya_ble_free(air_send_packet.encrypt_data_buf);
        return 1;
    }

    tuya_ble_free(air_send_packet.send_data);
    package_number = 0;
    trsmitr_init(&ty_trsmitr_proc_send);
    do
    {
        ret = trsmitr_send_pkg_encode(&ty_trsmitr_proc_send,TUYA_BLE_PROTOCOL_VERSION_HIGN,(uint8_t *)(air_send_packet.encrypt_data_buf), air_send_packet.encrypt_data_buf_len);
        if (MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret)
        {
            tuya_ble_free(air_send_packet.encrypt_data_buf);
            return 1;
        }
        send_len = get_trsmitr_subpkg_len(&ty_trsmitr_proc_send);
        memcpy(p_buf,get_trsmitr_subpkg(&ty_trsmitr_proc_send),send_len);
        package_number++;
        tuya_ble_gatt_send_data_enqueue(p_buf,send_len);

    } while (ret == MTP_TRSMITR_CONTINUE);

    TUYA_BLE_LOG_INFO("ble_commData_send len = %d , package_number = %d , protocol version : 0x%02x , error code : 0x%02x",air_send_packet.encrypt_data_buf_len,package_number,TUYA_BLE_PROTOCOL_VERSION_HIGN,err);

    tuya_ble_free(air_send_packet.encrypt_data_buf);

    return 0;
}



