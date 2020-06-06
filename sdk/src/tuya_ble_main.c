/**
 * \file tuya_ble_main.c
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
#include "tuya_ble_secure.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_sdk_version.h"
#include "tuya_ble_event.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_app_uart_common_handler.h"
#include "tuya_ble_app_production_test.h"
#include "tuya_ble_log.h"
#include "tuya_ble_internal_config.h"

tuya_ble_parameters_settings_t tuya_ble_current_para;


static volatile tuya_ble_connect_status_t tuya_ble_connect_status = UNKNOW_STATUS;

#if TUYA_BLE_USE_OS

uint32_t m_cb_queue_table[TUYA_BLE_MAX_CALLBACKS];

#else
tuya_ble_callback_t m_cb_table[TUYA_BLE_MAX_CALLBACKS];
#endif


void tuya_ble_connect_status_set(tuya_ble_connect_status_t status)
{
    tuya_ble_device_enter_critical();
    tuya_ble_connect_status = status;
    tuya_ble_device_exit_critical();

}

tuya_ble_connect_status_t tuya_ble_connect_status_get(void)
{
    return  tuya_ble_connect_status;
}





#if TUYA_BLE_USE_OS

#if TUYA_BLE_SELF_BUILT_TASK
void *tuya_ble_task_handle;   //!< APP Task handle
void *tuya_ble_queue_handle;  //!< Event queue handle

static void tuya_ble_main_task(void *p_param)
{
    tuya_ble_evt_param_t tuya_ble_evt;
    uint32_t i = 0;

    while (true)
    {
        if (tuya_ble_os_msg_queue_recv(tuya_ble_queue_handle, &tuya_ble_evt, 0xFFFFFFFF) == true)
        {
            tuya_ble_event_process(&tuya_ble_evt);

        }

    }

}
#endif

#else

void tuya_ble_main_tasks_exec(void)
{
    tuya_sched_execute();
}
#endif




void tuya_ble_event_init(void)
{
#if TUYA_BLE_USE_OS
    
#if TUYA_BLE_SELF_BUILT_TASK
    tuya_ble_os_task_create(&tuya_ble_task_handle, "tuya", tuya_ble_main_task, 0, TUYA_BLE_TASK_STACK_SIZE,TUYA_BLE_TASK_PRIORITY);
    tuya_ble_os_msg_queue_create(&tuya_ble_queue_handle, MAX_NUMBER_OF_TUYA_MESSAGE, sizeof(tuya_ble_evt_param_t));
#endif
    
#else
    tuya_ble_event_queue_init();
#endif

#if TUYA_BLE_USE_OS
    for(uint8_t i= 0; i<TUYA_BLE_MAX_CALLBACKS; i++)
    {
        m_cb_queue_table[i] = 0;
    }
     
#else
    for(uint8_t i= 0; i<TUYA_BLE_MAX_CALLBACKS; i++)
    {
        m_cb_table[i] = NULL;
    }
#endif

}


uint8_t tuya_ble_event_send(tuya_ble_evt_param_t *evt)
{
#if TUYA_BLE_USE_OS

#if TUYA_BLE_SELF_BUILT_TASK    
    if(tuya_ble_os_msg_queue_send(tuya_ble_queue_handle, evt, 0))
    {
        return 0;
    }
    else
    {
        return 1;
    }
#else
   if(tuya_ble_event_queue_send_port(evt,0))
   {
       return 0;
   }
   else
   {
       return 1;
   }   
#endif
    
#else
    if(tuya_ble_message_send(evt)==TUYA_BLE_SUCCESS)
    {
        return 0;
    }
    else
    {
        return 1;
    }
#endif
}


uint8_t tuya_ble_custom_event_send(tuya_ble_custom_evt_t evt)
{
    static tuya_ble_evt_param_t event;
    uint8_t ret = 0;

#if TUYA_BLE_USE_OS
    event.hdr.event = TUYA_BLE_EVT_CUSTOM;
    event.custom_evt = evt;
    
#if TUYA_BLE_SELT_BUILT_TASK    
    if(tuya_ble_os_msg_queue_send(tuya_ble_queue_handle, &event, 0))
    {
        return 0;
    }
    else
    {
        return 1;
    }
#else
   if(tuya_ble_event_queue_send_port(&event,0))
   {
       return 0;
   }
   else
   {
       return 1;
   }   
#endif  
    
#else
    tuya_ble_device_enter_critical();
    event.hdr.event = TUYA_BLE_EVT_CUSTOM;
    event.custom_evt = evt;

    if(tuya_ble_message_send(&event)==TUYA_BLE_SUCCESS)
    {
        ret = 0;
    }
    else
    {        
        ret = 1;
    }    
    tuya_ble_device_exit_critical();
    return ret;
#endif
}



tuya_ble_status_t tuya_ble_inter_event_response(tuya_ble_cb_evt_param_t *param)
{

    switch (param->evt)
    {
    case TUYA_BLE_CB_EVT_CONNECTE_STATUS:
        break;
    case TUYA_BLE_CB_EVT_DP_WRITE:
        if(param->dp_write_data.p_data)
        {
            tuya_ble_free(param->dp_write_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_DP_QUERY:
        if(param->dp_query_data.p_data)
        {
            tuya_ble_free(param->dp_query_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_OTA_DATA:
        if(param->ota_data.p_data)
        {
            tuya_ble_free(param->ota_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_NETWORK_INFO:
        if(param->network_data.p_data)
        {
            tuya_ble_free(param->network_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_WIFI_SSID:
        if(param->wifi_info_data.p_data)
        {
            tuya_ble_free(param->wifi_info_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_TIME_STAMP:
        break;
    case TUYA_BLE_CB_EVT_TIME_NORMAL:
        break;
    case TUYA_BLE_CB_EVT_DATA_PASSTHROUGH:

        if(param->ble_passthrough_data.p_data)
        {
            tuya_ble_free(param->ble_passthrough_data.p_data);
        }
        break;
    default:
        break;
    }

    return TUYA_BLE_SUCCESS;
}



uint8_t tuya_ble_cb_event_send(tuya_ble_cb_evt_param_t *evt)
{
#if TUYA_BLE_USE_OS
    if(m_cb_queue_table[0])
    {
        if(tuya_ble_os_msg_queue_send((void *)m_cb_queue_table[0], evt, 0))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
#else
    tuya_ble_callback_t fun;
    if(m_cb_table[0])
    {
        fun = m_cb_table[0];
        fun(evt);
        tuya_ble_inter_event_response(evt);
    }
#endif
    return 0;

}


#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==3)

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */

#define TUYA_BLE_ADV_DATA_LEN  (12+TUYA_BLE_PRODUCT_ID_MAX_LEN)

static const uint8_t adv_data_const[TUYA_BLE_ADV_DATA_LEN] =
{
    0x02,
    0x01,
    0x06,
    0x03,
    0x02,
    0x01, 0xA2,
    0x14,
    0x16,
    0x01, 0xA2,
    0x00,         //id type 00-pid 01-product key
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


#define TUYA_BLE_SCAN_RSP_DATA_LEN  30
static const uint8_t scan_rsp_data_const[TUYA_BLE_SCAN_RSP_DATA_LEN] =
{
    0x03,
    0x09,
    0x54, 0x59,
    0x19,             // length 
    0xFF,
    0xD0,
    0x07,
    0x00, //bond flag bit7 （8）
    0x03, //protocol version
    0x01, //Encry Mode （10）
    0x00,0x00, //communication way bit0-mesh bit1-wifi bit2-zigbee bit3-NB
    0x00, //data type
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static uint8_t adv_data[TUYA_BLE_ADV_DATA_LEN];
static uint8_t scan_rsp_data[TUYA_BLE_SCAN_RSP_DATA_LEN];

uint8_t tuya_ble_get_adv_connect_request_bit_status(void)
{
    return (scan_rsp_data[8]&0x01);
}

void tuya_ble_adv_change(void)
{
    uint8_t *aes_buf = NULL;
    uint8_t aes_key[16];
    uint8_t encry_device_id[DEVICE_ID_LEN];

    memcpy(adv_data,adv_data_const,TUYA_BLE_ADV_DATA_LEN);
    memcpy(&scan_rsp_data,scan_rsp_data_const,TUYA_BLE_SCAN_RSP_DATA_LEN);

    adv_data[7] = 4+tuya_ble_current_para.pid_len;
    adv_data[11] = tuya_ble_current_para.pid_type;
    
    scan_rsp_data[8] &=(~0x01);

    scan_rsp_data[9] = TUYA_BLE_PROTOCOL_VERSION_HIGN;

    scan_rsp_data[10] = TUYA_BLE_SECURE_CONNECTION_TYPE;

    scan_rsp_data[11] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY>>8;
    scan_rsp_data[12] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY;

    if(tuya_ble_current_para.sys_settings.bound_flag == 1)
    {
        scan_rsp_data[8] |=0x80 ;
        //
        memcpy(aes_key,tuya_ble_current_para.sys_settings.login_key,LOGIN_KEY_LEN);
        memcpy(aes_key+LOGIN_KEY_LEN,tuya_ble_current_para.auth_settings.device_id,16-LOGIN_KEY_LEN);

        aes_buf = tuya_ble_malloc(200);

        if(aes_buf==NULL)
        {
            return;
        }
        else
        {
            memset(aes_buf,0,200);
        }

        tuya_ble_encrypt_old_with_key(aes_key,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN,aes_buf);

        memcpy(&adv_data[12],(uint8_t *)(aes_buf+1),tuya_ble_current_para.pid_len);

        tuya_ble_free(aes_buf);

        memset(aes_key,0,sizeof(aes_key));
        memcpy(aes_key,&adv_data[12],tuya_ble_current_para.pid_len);

        tuya_ble_device_id_encrypt(aes_key,tuya_ble_current_para.pid_len,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN,encry_device_id);

        memcpy(&scan_rsp_data[14],encry_device_id,DEVICE_ID_LEN);

    }
    else
    {
        scan_rsp_data[8] &=(~0x80);

        memcpy(&adv_data[12],tuya_ble_current_para.pid,tuya_ble_current_para.pid_len);
        tuya_ble_device_id_encrypt(tuya_ble_current_para.pid,tuya_ble_current_para.pid_len,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN,encry_device_id);

        memcpy(&scan_rsp_data[14],encry_device_id,DEVICE_ID_LEN);
    }
    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d",tuya_ble_current_para.sys_settings.bound_flag);
    tuya_ble_gap_advertising_adv_data_update(adv_data,tuya_ble_current_para.pid_len+12);
    tuya_ble_gap_advertising_scan_rsp_data_update(scan_rsp_data,sizeof(scan_rsp_data));

}

void tuya_ble_adv_change_with_connecting_request(void)
{
    uint8_t *aes_buf = NULL;
    uint8_t aes_key[16];
    uint8_t encry_device_id[DEVICE_ID_LEN];

    memcpy(adv_data,adv_data_const,TUYA_BLE_ADV_DATA_LEN);
    memcpy(&scan_rsp_data,scan_rsp_data_const,TUYA_BLE_SCAN_RSP_DATA_LEN);

    adv_data[7] = 4+tuya_ble_current_para.pid_len;
    adv_data[11] = tuya_ble_current_para.pid_type;

    scan_rsp_data[8] |= 0x01 ;
    
    scan_rsp_data[9] = TUYA_BLE_PROTOCOL_VERSION_HIGN;

    scan_rsp_data[10] = TUYA_BLE_SECURE_CONNECTION_TYPE;

    scan_rsp_data[11] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY>>8;
    scan_rsp_data[12] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY;

    if(tuya_ble_current_para.sys_settings.bound_flag == 1)
    {
        scan_rsp_data[8] |=0x80 ;
        //
        memcpy(aes_key,tuya_ble_current_para.sys_settings.login_key,LOGIN_KEY_LEN);
        memcpy(aes_key+LOGIN_KEY_LEN,tuya_ble_current_para.auth_settings.device_id,16-LOGIN_KEY_LEN);

        aes_buf = tuya_ble_malloc(200);

        if(aes_buf==NULL)
        {
            return;
        }
        else
        {
            memset(aes_buf,0,200);
        }

        tuya_ble_encrypt_old_with_key(aes_key,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN,aes_buf);

        memcpy(&adv_data[12],(uint8_t *)(aes_buf+1),tuya_ble_current_para.pid_len);

        tuya_ble_free(aes_buf);

        memset(aes_key,0,sizeof(aes_key));
        memcpy(aes_key,&adv_data[12],tuya_ble_current_para.pid_len);

        tuya_ble_device_id_encrypt(aes_key,tuya_ble_current_para.pid_len,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN,encry_device_id);

        memcpy(&scan_rsp_data[14],encry_device_id,DEVICE_ID_LEN);

    }
    else
    {
        scan_rsp_data[8] &=(~0x80);

        memcpy(&adv_data[12],tuya_ble_current_para.pid,tuya_ble_current_para.pid_len);
        tuya_ble_device_id_encrypt(tuya_ble_current_para.pid,tuya_ble_current_para.pid_len,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN,encry_device_id);


        memcpy(&scan_rsp_data[14],encry_device_id,DEVICE_ID_LEN);
    }
    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d",tuya_ble_current_para.sys_settings.bound_flag);
    tuya_ble_gap_advertising_adv_data_update(adv_data,tuya_ble_current_para.pid_len+12);
    tuya_ble_gap_advertising_scan_rsp_data_update(scan_rsp_data,sizeof(scan_rsp_data));

}

#else

/** @brief  GAP - scan response data (max size = 31 bytes) */
static const uint8_t scan_rsp_data_const[30] =
{
    /* Manufacturer Specific Data */
    0x1D,             /* length */
    0xFF,
    0x59,
    0x02,
    0x00,0x02,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};


/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */
static const uint8_t adv_data_const[20] =
{
    /* Flags */
    0x02,             /* length */
    0x01, /* type="Flags" */
    0x05,
    /* Local name */
    0x0C,             /* length */
    0x09,
    'T', 'U', 'Y', 'A', '_', 'C', 'O', 'M', 'M', 'O','N',
    0x03,
    0x02,
    0x01, 0xA2,
};



void tuya_ble_adv_change(void)
{
    static uint8_t aes_buf[200];
    uint8_t aes_key[16];
    uint8_t encry_device_id[DEVICE_ID_LEN];

    //scanrsp
    uint8_t scan_rsp_data[30];
    memcpy(&scan_rsp_data,scan_rsp_data_const,sizeof(scan_rsp_data_const));

    scan_rsp_data[4] &=(~0x70);

    if(tuya_ble_current_para.sys_settings.bound_flag == 1)
    {
        scan_rsp_data[4] |=0x80 ;
        //
        memcpy(aes_key,tuya_ble_current_para.sys_settings.login_key,LOGIN_KEY_LEN);
        memcpy(aes_key+LOGIN_KEY_LEN,tuya_ble_current_para.auth_settings.device_id,16-LOGIN_KEY_LEN);

        tuya_ble_encrypt_old_with_key(aes_key,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN,aes_buf);

        memcpy(&scan_rsp_data[6],(uint8_t *)(aes_buf+1),8);

        memset(aes_key,0,sizeof(aes_key));
        memcpy(aes_key,&scan_rsp_data[6],8);

        tuya_ble_device_id_encrypt(aes_key,8,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN,encry_device_id);

        memcpy(&scan_rsp_data[6+8],encry_device_id,DEVICE_ID_LEN);

    }
    else
    {
        scan_rsp_data[4] &=(~0x80);
        memcpy(&scan_rsp_data[6],tuya_ble_current_para.pid,8);
        tuya_ble_device_id_encrypt(tuya_ble_current_para.pid,8,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN,encry_device_id);

        memcpy(&scan_rsp_data[6+8],encry_device_id,DEVICE_ID_LEN);
    }
    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d",tuya_ble_current_para.sys_settings.bound_flag);

    tuya_ble_gap_advertising_scan_rsp_data_update(scan_rsp_data,sizeof(scan_rsp_data));

}

#endif



