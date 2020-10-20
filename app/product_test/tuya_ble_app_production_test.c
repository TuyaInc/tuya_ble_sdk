/**
 * \file tuya_ble_app_production_test.c
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
#include "tuya_ble_app_production_test.h"
#include "tuya_ble_log.h"
#include "tuya_ble_api.h"


#if defined(CUSTOMIZED_TUYA_BLE_APP_PRODUCT_TEST_HEADER_FILE)
#include CUSTOMIZED_TUYA_BLE_APP_PRODUCT_TEST_HEADER_FILE
#endif


#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE&&TUYA_BLE_DEVICE_AUTH_DATA_STORE)

static uint8_t tuya_ble_production_test_flag = 0;

static uint8_t tuya_ble_production_test_with_ble_flag = 0;

static uint8_t tuya_ble_storage_write_auth_key_device_id_mac_flag = 0;

static uint8_t tuya_ble_storage_write_hid_flag = 0;

#define tuya_ble_prod_monitor_timeout_ms  60000  //60s

tuya_ble_timer_t tuya_ble_xTimer_prod_monitor;


void tuya_ble_internal_production_test_with_ble_flag_clear(void)
{
    tuya_ble_production_test_with_ble_flag = 0;
}


static void tuya_ble_vtimer_prod_monitor_callback(tuya_ble_timer_t pxTimer)
{
    tuya_ble_device_delay_ms(1000);
    tuya_ble_device_reset();

}

static void tuya_ble_prod_monitor_timer_init(void)
{
    if(tuya_ble_timer_create(&tuya_ble_xTimer_prod_monitor,tuya_ble_prod_monitor_timeout_ms, TUYA_BLE_TIMER_SINGLE_SHOT,tuya_ble_vtimer_prod_monitor_callback) != TUYA_BLE_SUCCESS)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_xTimer_prod_monitor creat failed");
    }

}


static void tuya_ble_prod_monitor_timer_start(void)
{
    if(tuya_ble_timer_start(tuya_ble_xTimer_prod_monitor) != TUYA_BLE_SUCCESS)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_xTimer_prod_monitor start failed");
    }

}

static void tuya_ble_prod_monitor_timer_stop(void)
{

    if(tuya_ble_timer_stop(tuya_ble_xTimer_prod_monitor) != TUYA_BLE_SUCCESS)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_xTimer_prod_monitor stop failed");
    }
    
}


static uint32_t tuya_ble_uart_prod_send(uint8_t type,uint8_t *pdata,uint8_t len)
{
    uint8_t uart_send_len = 7+len;
    uint8_t *uart_send_buffer = NULL;
    
    uart_send_buffer=(uint8_t *)tuya_ble_malloc(uart_send_len);
    if(uart_send_buffer!=NULL)
    {
        uart_send_buffer[0] = 0x66;
        uart_send_buffer[1] = 0xAA;
        uart_send_buffer[2] = 0x00;
        uart_send_buffer[3] = type;
        uart_send_buffer[4] = 0;
        uart_send_buffer[5] = len;        
        memcpy(uart_send_buffer+6,pdata,len);
        uart_send_buffer[6+len] = tuya_ble_check_sum(uart_send_buffer,6+len);
        tuya_ble_common_uart_send_data(uart_send_buffer,7+len);
        tuya_ble_free(uart_send_buffer);
    }
    else
    {
        TUYA_BLE_LOG_ERROR("uart prod send buffer malloc failed.");    
        return 1;        
    }

    return 0;
}


__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_prod_beacon_scan_start(void)
{
    return TUYA_BLE_SUCCESS;
}

__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_prod_beacon_scan_stop(void)
{
    return TUYA_BLE_SUCCESS;
}

__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_prod_beacon_get_rssi_avg(int8_t *rssi)
{
    *rssi = -20;
    return TUYA_BLE_SUCCESS;
}


static void tuya_ble_auc_enter(uint8_t *para, uint16_t len)
{
    uint8_t buf[1];

    TUYA_BLE_LOG_DEBUG("AUC ENTER!");
    
    buf[0] = (TUYA_BLE_AUC_FINGERPRINT_VER<<TUYA_BLE_AUC_FW_FINGERPRINT_POS)|(TUYA_BLE_AUC_WRITE_PID<<TUYA_BLE_AUC_WRITE_PID_POS);

    if(tuya_ble_production_test_flag == 1)
    {
        //tuya_ble_stop_escan();
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_ENTER,buf,1);
        return;
    }
    tuya_ble_prod_monitor_timer_init();
    
    tuya_ble_prod_monitor_timer_start();
    
    tuya_ble_prod_beacon_scan_start();

    tuya_ble_production_test_flag = 1;
    
    tuya_ble_storage_write_auth_key_device_id_mac_flag = 0;

    tuya_ble_storage_write_hid_flag = 0;

    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_ENTER,buf,1);
}


static  void tuya_ble_auc_query_hid(uint8_t *para, uint16_t len)
{
    if(tuya_ble_production_test_flag != 1)
    {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC QUERY HID!");
    char buf[70] = "{\"ret\":true,\"hid\":\"\"}";


    if(tuya_ble_buffer_value_is_all_x(tuya_ble_current_para.auth_settings.h_id,H_ID_LEN,0xFF))
    {
        buf[19] = '\"';
        buf[20] = '}';
    }
    else if(tuya_ble_buffer_value_is_all_x(tuya_ble_current_para.auth_settings.h_id,H_ID_LEN,0))
    {
        buf[19] = '\"';
        buf[20] = '}';
    }
    else
    {
        memcpy(&buf[19],tuya_ble_current_para.auth_settings.h_id,H_ID_LEN);
        buf[38] = '\"';
        buf[39] = '}';
    }

    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_QUERY_HID,(uint8_t *)buf,strlen(buf));
    
    TUYA_BLE_LOG_HEXDUMP_DEBUG("AUC QUERY HID response data : ",(uint8_t *)buf,strlen(buf));
}



__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_prod_gpio_test(void)
{
    return TUYA_BLE_SUCCESS;
}


static void tuya_ble_auc_gpio_test(uint8_t *para, uint16_t len)
{
    char ture_buf[] = "{\"ret\":true}";
    char false_buf[] = "{\"ret\":false}";
    if(tuya_ble_production_test_flag != 1)
    {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC GPIO TEST!");

    if(tuya_ble_prod_gpio_test() == TUYA_BLE_SUCCESS)
    {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_GPIO_TEST,(uint8_t *)ture_buf,strlen(ture_buf));
        TUYA_BLE_LOG_DEBUG("AUC GPIO TEST successed!");
    }
    else
    {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_GPIO_TEST,(uint8_t *)false_buf,strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC GPIO TEST failed!");
    }

}


static void tuya_ble_prod_asciitohex(uint8_t *ascbuf,uint8_t len,uint8_t *hexbuf)
{
    uint8_t i =0,j =0;

    for(j = 0; j<(len/2); j++)
    {       
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
    
}

static void tuya_ble_storage_write_auth_key_device_id_mac_async_completed(void *p_param,tuya_ble_status_t result)
{
    char true_buf[] = "{\"ret\":true}";
    char false_buf[] = "{\"ret\":false}";
    
    if(result==TUYA_BLE_SUCCESS)
    {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO,(uint8_t *)true_buf,strlen(true_buf));
        TUYA_BLE_LOG_DEBUG("AUC WRITE AUTH INFO successed!"); 
    }
    else
    {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO,(uint8_t *)false_buf,strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC_CMD_WRITE_AUTH_INFO failed!");
    }
    tuya_ble_storage_write_auth_key_device_id_mac_flag = 0;
}

static  void tuya_ble_auc_write_auth_info(uint8_t *para, uint16_t len)
{
    uint8_t mac_temp[6];
    uint8_t mac_char[13];
    char true_buf[] = "{\"ret\":true}";
    char false_buf[] = "{\"ret\":false}";
    
    if(tuya_ble_production_test_flag != 1)
    {
        return;
    }
    
    TUYA_BLE_LOG_DEBUG("AUC WRITE AUTH INFO!");    
        
    /*
      {
      "auzkey":"xxxx",    //"6":"32",         7   +  6+4
      "uuid":"xxxx",      //"4":"16",         7   +6+32+6   +    4+4
      "mac":"xxxxxx",     //"3":"12",
      "prod_test":"xxxx"    //"9":"4/5"
      }
      */
   
    if(len<100)
    {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO,(uint8_t *)false_buf,strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC_CMD_WRITE_AUTH_INFO error ,since Invalid length!");
        return;
    }
    
    if((memcmp(&para[2],"auzkey",6)!=0)||(memcmp(&para[46],"uuid",4)!=0)||(memcmp(&para[72],"mac",3)!=0))
    {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO,(uint8_t *)false_buf,strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC_CMD_WRITE_AUTH_INFO error ,since Invalid paras");
        return;
    }
    
    memcpy(mac_char,&para[78],12);
    tuya_ble_prod_asciitohex(mac_char,12,mac_temp);

    if(tuya_ble_storage_write_auth_key_device_id_mac_flag)
    {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO,(uint8_t *)false_buf,strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC_CMD_WRITE_AUTH_INFO busy!");
    }
    else
    {
        tuya_ble_storage_write_auth_key_device_id_mac_flag = 1;
        tuya_ble_storage_write_auth_key_device_id_mac_async(&para[11],AUTH_KEY_LEN,&para[53],DEVICE_ID_LEN,mac_temp,MAC_LEN,mac_char,MAC_LEN*2,tuya_ble_storage_write_auth_key_device_id_mac_async_completed);
    }
           
}




static void tuya_ble_auc_query_info(uint8_t *para, uint16_t len)
{
    
    uint8_t i=0;
    uint8_t mac_temp[13];
    uint8_t *alloc_buf = NULL;
    
    if(tuya_ble_production_test_flag != 1)
    {
        return;
    }    
       
    TUYA_BLE_LOG_DEBUG("AUC QUERY INFO!"); 

    alloc_buf = (uint8_t *)tuya_ble_malloc(256);
    
    if(alloc_buf)
    {
        memset(alloc_buf,0,256);  
    }   
    else
    {
        TUYA_BLE_LOG_ERROR("AUC QUERY INFO alloc buf malloc failed."); 
        return;
    }        
    
    alloc_buf[i++] = '{';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i],"ret",3);
    i += 3;
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ':';
    memcpy(&alloc_buf[i],"true",4);
    i += 4;

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i],"auzKey",6);
    i += 6;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i],tuya_ble_current_para.auth_settings.auth_key,AUTH_KEY_LEN);
    i += AUTH_KEY_LEN;
    
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i],"hid",3);
    i += 3;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';
    alloc_buf[i++] = '\"';
    memcpy( &alloc_buf[i],tuya_ble_current_para.auth_settings.h_id,H_ID_LEN);
    i += 19;
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i],"uuid",4);
    i += 4;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';
    alloc_buf[i++] = '\"';
    memcpy( &alloc_buf[i],tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN);
    i += DEVICE_ID_LEN;
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i],"mac",3);
    i += 3;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';
    alloc_buf[i++] = '\"';

    memcpy( &alloc_buf[i],tuya_ble_current_para.auth_settings.mac_string,MAC_LEN*2);
    i += MAC_LEN*2;
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i],"firmName",8);
    i += 8;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i],TUYA_BLE_APP_BUILD_FIRMNAME_STRING,strlen(TUYA_BLE_APP_BUILD_FIRMNAME_STRING));
    i+=strlen(TUYA_BLE_APP_BUILD_FIRMNAME_STRING);
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i],"firmVer",7);
    i+=7;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i],TUYA_BLE_APP_VERSION_STRING,strlen(TUYA_BLE_APP_VERSION_STRING));
    i+=strlen(TUYA_BLE_APP_VERSION_STRING);
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i],"prod_test",9);
    i+=9;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';

    memcpy(&alloc_buf[i],"false",5);
    i += 5;

    alloc_buf[i++] = '}';
    
    alloc_buf[i++] = 0;

    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_QUERY_INFO,(uint8_t *)alloc_buf,i-1);
    
    TUYA_BLE_LOG_DEBUG("AUC_CMD_QUERY_INFO RESPONSE!");
    
    tuya_ble_free(alloc_buf);
}


static void tuya_ble_auc_reset(uint8_t *para, uint16_t len)
{
    char buf[] = "{\"ret\":true}";
    if(tuya_ble_production_test_flag != 1)
    {
        return;
    }  
    TUYA_BLE_LOG_DEBUG("auc RESET!");   
    
    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_RESET,(uint8_t *)buf,strlen(buf));
    
    tuya_ble_device_delay_ms(1000);
    
    tuya_ble_device_reset();
    
}



static void tuya_ble_storage_write_hid_async_completed(void *p_param,tuya_ble_status_t result)
{
    char true_buf[] = "{\"ret\":true}";
    char false_buf[] = "{\"ret\":false}";
    
    if(result==TUYA_BLE_SUCCESS)
    {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_HID,(uint8_t *)true_buf,strlen(true_buf));
        TUYA_BLE_LOG_DEBUG("WRITE AUTH HID successed."); 
    }
    else
    {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_HID,(uint8_t *)false_buf,strlen(false_buf));
        TUYA_BLE_LOG_ERROR("WRITE AUTH HID failed."); 
    }
    
    tuya_ble_storage_write_hid_flag = 0;
    
}

static  void tuya_ble_auc_write_hid(uint8_t *para, uint16_t len)
{
    uint8_t hid[19];
    char true_buf[] = "{\"ret\":true}";
    char false_buf[] = "{\"ret\":false}";
    
    if(tuya_ble_production_test_flag != 1)
    {
        return;
    }
    
    TUYA_BLE_LOG_DEBUG("AUC WRITE AUTH HID!");    
    
    if(len<27)
    {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_HID,(uint8_t *)false_buf,strlen(false_buf));
        TUYA_BLE_LOG_ERROR("WRITE AUTH HID para length error!"); 
        return;
    }
    
    if(memcmp(&para[2],"hid",3)!=0)
    {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_HID,(uint8_t *)false_buf,strlen(false_buf));
        TUYA_BLE_LOG_ERROR("WRITE AUTH HID para error!"); 
        return;
    }
           
	memcpy(hid,&para[8],H_ID_LEN);
	
    if(tuya_ble_storage_write_hid_flag)
    {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_HID,(uint8_t *)false_buf,strlen(false_buf));
        TUYA_BLE_LOG_ERROR("WRITE AUTH HID busy."); 
    }
    else
    {
        tuya_ble_storage_write_hid_flag = 1;
        tuya_ble_storage_write_hid_async(hid,H_ID_LEN,tuya_ble_storage_write_hid_async_completed); 
    }        
}


static void tuya_ble_auc_query_fingerprint(uint8_t *para, uint16_t len)
{   
    int32_t length = 0;
    uint8_t *alloc_buf = NULL;
    
    if(tuya_ble_production_test_flag != 1)
    {
        return;
    }    
    
    TUYA_BLE_LOG_DEBUG("AUC QUERY FINGERPRINT!");
    
    alloc_buf = (uint8_t *)tuya_ble_malloc(256);
    
    if(alloc_buf)
    {
        memset(alloc_buf,0,256);  
    }   
    else
    {
        TUYA_BLE_LOG_ERROR("AUC QUERY INFO alloc buf malloc failed."); 
        return;
    } 
        
    length = sprintf((char *)alloc_buf,"{\"ret\":true,\"firmName\":\"%s\",\"firmVer\":\"%s\"}",TUYA_BLE_APP_BUILD_FIRMNAME_STRING,TUYA_BLE_APP_VERSION_STRING);
    
    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_QUERY_FINGERPRINT,alloc_buf,length);
    
    tuya_ble_free(alloc_buf);
    
    TUYA_BLE_LOG_DEBUG("AUC_CMD_QUERY_FINGERPRINT responsed."); 
    
}




static void tuya_ble_auc_rssi_test(uint8_t *para, uint16_t len)
{
    uint8_t length = 0;
    int8_t rssi = 0;
    static const char false_buf[] = "{\"ret\":false}";
    char true_buf[30];
    
    if(tuya_ble_production_test_flag != 1)
    {
        return;
    }     
    
    TUYA_BLE_LOG_DEBUG("AUC RSSI TEST!");   
    
    memset(true_buf,0,sizeof(true_buf));  
    
    tuya_ble_prod_beacon_scan_stop();
    
    if(tuya_ble_prod_beacon_get_rssi_avg(&rssi) != TUYA_BLE_SUCCESS)
    {       
        TUYA_BLE_LOG_ERROR("auc get rssi failed.");
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_RSSI_TEST,(uint8_t *)false_buf,strlen(false_buf));
    }
    else
    {        
        length = sprintf((char *)true_buf,"{\"ret\":true,\"rssi\":\"%d\"}",rssi);
        TUYA_BLE_LOG_DEBUG("auc get rssi = %d",rssi);
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_RSSI_TEST,(uint8_t *)true_buf,length);
    }
}


__TUYA_BLE_WEAK void tuya_ble_custom_app_production_test_process(uint8_t channel,uint8_t *p_in_data,uint16_t in_len)
{
    uint16_t cmd = 0;
    uint8_t *data_buffer = NULL;
    uint16_t data_len = ((p_in_data[4]<<8) + p_in_data[5]);
       
    if((p_in_data[6] != 3)||(data_len<3))
        return;
    
    cmd = (p_in_data[7]<<8) + p_in_data[8];
    data_len -= 3;
    if(data_len>0)
    {
        data_buffer = p_in_data+9;
    }
    
    switch(cmd)
    {   

        
        default:
            break;
    };    
    
    
}

extern void tuya_ble_connect_monitor_timer_stop(void);
void tuya_ble_app_production_test_process(uint8_t channel,uint8_t *p_in_data,uint16_t in_len)
{
    uint8_t cmd = p_in_data[3];
    uint16_t data_len = (p_in_data[4]<<8) + p_in_data[5];
    uint8_t *data_buffer = p_in_data+6;
   /* 
    if(tuya_ble_current_para.sys_settings.factory_test_flag==0) //
    {
        TUYA_BLE_LOG_WARNING("The production interface is closed!");
        return;
    }
    */
    if((channel!=0)&&(cmd!=TUYA_BLE_AUC_CMD_EXTEND))
    {
        TUYA_BLE_LOG_ERROR("The authorization instructions are not supported in non-serial channels!");
        return;
    }
    if((channel==1)&&(cmd==TUYA_BLE_AUC_CMD_EXTEND))
    {
        if(tuya_ble_production_test_with_ble_flag==0)
        {
            tuya_ble_production_test_with_ble_flag = 1;
            if(tuya_ble_connect_status_get()!=BONDING_CONN)
            {
                tuya_ble_connect_monitor_timer_stop();
            }
        }
        
    }
    switch(cmd)
    {
        case TUYA_BLE_AUC_CMD_EXTEND:
            tuya_ble_custom_app_production_test_process(channel,p_in_data,in_len);
            break;
        case TUYA_BLE_AUC_CMD_ENTER:
            tuya_ble_auc_enter(data_buffer,data_len);
            break;
        case TUYA_BLE_AUC_CMD_QUERY_HID:
            tuya_ble_auc_query_hid(data_buffer,data_len);
            break;
        case TUYA_BLE_AUC_CMD_GPIO_TEST:
            tuya_ble_auc_gpio_test(data_buffer,data_len);
            break;
        case TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO:
            tuya_ble_auc_write_auth_info(data_buffer,data_len);
            break;
        case TUYA_BLE_AUC_CMD_QUERY_INFO:
            tuya_ble_auc_query_info(data_buffer,data_len);
            break;
        case TUYA_BLE_AUC_CMD_RESET:
            tuya_ble_auc_reset(data_buffer,data_len);
            break;
        case TUYA_BLE_AUC_CMD_QUERY_FINGERPRINT:
            tuya_ble_auc_query_fingerprint(data_buffer,data_len);
            break;
        case TUYA_BLE_AUC_CMD_WRITE_HID:
            tuya_ble_auc_write_hid(data_buffer,data_len);
            break;
        case TUYA_BLE_AUC_CMD_RSSI_TEST:
            tuya_ble_auc_rssi_test(data_buffer,data_len);
            break;
        case TUYA_BLE_AUC_CMD_WRITE_OEM_INFO:
            
            break;
                
        default:
            break;
    };
    
        
}

#else

void tuya_ble_app_production_test_process(uint8_t channel,uint8_t *p_in_data,uint16_t in_len)
{
    uint8_t cmd = p_in_data[3];
    uint16_t data_len = (p_in_data[4]<<8) + p_in_data[5];
    uint8_t *data_buffer = p_in_data+6;
    switch(cmd)
    {  
        default:
            break;
    };
    
        
}

#endif



