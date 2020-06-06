/**
 * \file tuya_ble_type.h
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


#ifndef TUYA_BLE_TYPE_H__
#define TUYA_BLE_TYPE_H__

#include "tuya_ble_stdlib.h"


#ifndef NULL
#define NULL 0
#endif

#if defined(__CC_ARM)
#pragma anon_unions
#elif defined(__ICCARM__)
#pragma language = extended
#elif defined(__GNUC__)
/* anonymous unions are enabled by default */
#endif


#if defined ( __CC_ARM )

    #ifndef __TUYA_BLE_ASM
        #define __TUYA_BLE_ASM               __asm
    #endif

    #ifndef __TUYA_BLE_INLINE
        #define __TUYA_BLE_INLINE            __inline
    #endif

    #ifndef __TUYA_BLE_WEAK
        #define __TUYA_BLE_WEAK              __weak
    #endif

    #ifndef __TUYA_BLE_ALIGN
        #define __TUYA_BLE_ALIGN(n)          __align(n)
    #endif

    #ifndef __TUYA_BLE_PACKED
        #define __TUYA_BLE_PACKED            __packed
    #endif

    #define TUYA_BLE_GET_SP()                __current_sp()

#elif defined ( __ICCARM__ )

    #ifndef __TUYA_BLE_ASM
        #define __TUYA_BLE_ASM               __asm
    #endif

    #ifndef __TUYA_BLE_INLINE
        #define __TUYA_BLE_INLINE            inline
    #endif

    #ifndef __TUYA_BLE_WEAK
        #define __TUYA_BLE_WEAK              __weak
    #endif

    #ifndef __TUYA_BLE_ALIGN
        #define TUYA_BLE_STRING_PRAGMA(x) _Pragma(#x)
        #define __TUYA_BLE_ALIGN(n) STRING_PRAGMA(data_alignment = n)
    #endif

    #ifndef __TUYA_BLE_PACKED
        #define __TUYA_BLE_PACKED            __packed
    #endif
    
    #define TUYA_BLE_GET_SP()                __get_SP()

#elif defined   ( __GNUC__ )

    #ifndef __TUYA_BLE_ASM
        #define __TUYA_BLE_ASM               __asm
    #endif

    #ifndef __TUYA_BLE_INLINE
        #define __TUYA_BLE_INLINE            inline
    #endif

    #ifndef __TUYA_BLE_WEAK
        #define __TUYA_BLE_WEAK              __attribute__((weak))
    #endif

    #ifndef __TUYA_BLE_ALIGN
        #define __TUYA_BLE_ALIGN(n)          __attribute__((aligned(n)))
    #endif

    #ifndef __TUYA_BLE_PACKED
        #define __TUYA_BLE_PACKED           __attribute__((packed)) 
    #endif

    #define TUYA_BLE_GET_SP()                tuya_ble_gcc_current_sp()

    static inline unsigned int tuya_ble_gcc_current_sp(void)
    {
        register unsigned sp __asm("sp");
        return sp;
    }
#endif


#define TUYA_BLE_EVT_BASE       0x00
#define TUYA_BLE_CB_EVT_BASE    0x40

#define H_ID_LEN              19
#define TUYA_BLE_PRODUCT_ID_DEFAULT_LEN        8
//#define PRODUCT_KEY_LEN       TUYA_BLE_PRODUCT_ID_OR_KEY_LEN
#define DEVICE_ID_LEN         16
#define DEVICE_ID_LEN_MAX     20
#define AUTH_KEY_LEN          32
#define LOGIN_KEY_LEN         6
#define ECC_SECRET_KEY_LEN    32
#define DEVICE_VIRTUAL_ID_LEN 22
#define SECRET_KEY_LEN        16
#define PAIR_RANDOM_LEN       6
#define MAC_LEN               6

#define TUYA_BLE_PRODUCT_ID_MAX_LEN  16

/** @defgroup TUY_BLE_DEVICE_COMMUNICATION_ABILITY tuya ble device communication ability
 * @{
 */
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE                0x0000
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_REGISTER_FROM_BLE  0x0001
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_MESH               0x0002
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_24G           0x0004
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_5G            0x0008
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_ZIGBEE             0x0010
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_NB                 0x0020
/** End of TUY_BLE_DEVICE_COMMUNICATION_ABILITY
  * @}
  */

#define TUYA_BLE_LOG_LEVEL_ERROR        1U
#define TUYA_BLE_LOG_LEVEL_WARNING      2U
#define TUYA_BLE_LOG_LEVEL_INFO         3U
#define TUYA_BLE_LOG_LEVEL_DEBUG        4U

#define TUYA_APP_LOG_LEVEL_ERROR        1U
#define TUYA_APP_LOG_LEVEL_WARNING      2U
#define TUYA_APP_LOG_LEVEL_INFO         3U
#define TUYA_APP_LOG_LEVEL_DEBUG        4U

/** @defgroup TUYA_BLE_SECURE_CONNECTION_TYPE tuya ble secure connection type
 * @{
 */
#define TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY     0x00

#define TUYA_BLE_SECURE_CONNECTION_WITH_ECC          0x01

#define TUYA_BLE_SECURE_CONNECTION_WTIH_PASSTHROUGH  0x02

#define TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_DEVCIE_ID_20     0x03
/** End of TUYA_BLE_SECURE_CONNECTION_TYPE
  * @}
  */

typedef enum {
    TUYA_BLE_SUCCESS  = 0x00,
    TUYA_BLE_ERR_INTERNAL,
    TUYA_BLE_ERR_NOT_FOUND,
    TUYA_BLE_ERR_NO_EVENT,
    TUYA_BLE_ERR_NO_MEM,
    TUYA_BLE_ERR_INVALID_ADDR,     // Invalid pointer supplied
    TUYA_BLE_ERR_INVALID_PARAM,    // Invalid parameter(s) supplied.
    TUYA_BLE_ERR_INVALID_STATE,    // Invalid state to perform operation.
    TUYA_BLE_ERR_INVALID_LENGTH,
    TUYA_BLE_ERR_DATA_SIZE,
    TUYA_BLE_ERR_TIMEOUT,
    TUYA_BLE_ERR_BUSY,
    TUYA_BLE_ERR_COMMON,
    TUYA_BLE_ERR_RESOURCES,
    TUYA_BLE_ERR_UNKNOWN,          // other ble sdk errors
} tuya_ble_status_t;



typedef enum{
    TUYA_BLE_PRODUCT_ID_TYPE_PID,
    TUYA_BLE_PRODUCT_ID_TYPE_PRODUCT_KEY,   
}tuya_ble_product_id_type_t;


typedef enum {
    TUYA_BLE_ADDRESS_TYPE_PUBLIC, // public address
    TUYA_BLE_ADDRESS_TYPE_RANDOM, // random address
} tuya_ble_addr_type_t;

typedef struct {
    tuya_ble_addr_type_t addr_type;
    uint8_t addr[6];    
}tuya_ble_gap_addr_t;

typedef struct{
    uint8_t device_id_len;   //if ==20,Compressed into 16
	uint8_t device_id[DEVICE_ID_LEN_MAX];
    tuya_ble_product_id_type_t p_type;
    uint8_t product_id_len;
	uint8_t product_id[TUYA_BLE_PRODUCT_ID_MAX_LEN];
    uint8_t device_vid[DEVICE_VIRTUAL_ID_LEN];
	uint8_t auth_key[AUTH_KEY_LEN];
	uint8_t login_key[LOGIN_KEY_LEN];
    uint8_t bound_flag;
    uint32_t firmware_version; //0x00010102 : v1.1.2
    uint32_t hardware_version;
    uint8_t reserve_1;
    uint8_t reserve_2;
}tuya_ble_device_param_t;	


typedef struct {
    uint16_t min_conn_interval;    // Range: 0x0006 to 0x0C80, Time = N * 1.25 msec, Time Range: 7.5 msec to 4 seconds.
    uint16_t max_conn_interval;    // Range: 0x0006 to 0x0C80, Time = N * 1.25 msec, Time Range: 7.5 msec to 4 seconds.
    uint16_t slave_latency;        // Range: 0x0000 to 0x01F3
    uint16_t conn_sup_timeout;     // Range: 0x000A to 0x0C80, Time = N * 10 msec, Time Range: 100 msec to 32 seconds
} tuya_ble_gap_conn_param_t;


typedef enum {
	TUYA_BLE_EVT_MTU_DATA_RECEIVE = TUYA_BLE_EVT_BASE,
    TUYA_BLE_EVT_DEVICE_INFO_UPDATE,
	TUYA_BLE_EVT_DP_DATA_REPORTED,
	TUYA_BLE_EVT_DP_DATA_WITH_TIME_REPORTED,
    TUYA_BLE_EVT_DP_DATA_WITH_TIME_STRING_REPORTED,
	TUYA_BLE_EVT_FACTORY_RESET,
	TUYA_BLE_EVT_OTA_RESPONSE,
    TUYA_BLE_EVT_DATA_PASSTHROUGH,
    TUYA_BLE_EVT_PRODUCTION_TEST_RESPONSE,
	TUYA_BLE_EVT_UART_CMD,
	TUYA_BLE_EVT_BLE_CMD,
    TUYA_BLE_EVT_NET_CONFIG_RESPONSE,
    TUYA_BLE_EVT_CUSTOM,
    TUYA_BLE_EVT_CONNECT_STATUS_UPDATE,
    TUYA_BLE_EVT_UNBOUND_RESPONSE,
    TUYA_BLE_EVT_ANOMALY_UNBOUND_RESPONSE,
    TUYA_BLE_EVT_DEVICE_RESET_RESPONSE,
    TUYA_BLE_EVT_TIME_REQ,
	TUYA_BLE_EVT_GATT_SEND_DATA,
    TUYA_BLE_EVT_CONNECTING_REQUEST,
    TUYA_BLE_EVT_SAVE_SYS_SETTINGS_DATA_CALLBACK,
} tuya_ble_evt_t;


typedef struct{
	uint8_t data[20];
	uint8_t len;
}tuya_ble_mtu_data_receive_t;    




typedef enum{
    DEVICE_INFO_TYPE_PID,
    DEVICE_INFO_TYPE_PRODUCT_KEY,
    DEVICE_INFO_TYPE_LOGIN_KEY,
    DEVICE_INFO_TYPE_BOUND,    
}tuya_ble_device_info_type_t;


typedef struct{
	tuya_ble_device_info_type_t type;
	uint8_t len;
    uint8_t data[32];
}tuya_ble_device_info_data_t;  



typedef struct{
	uint8_t *p_data;
	uint16_t data_len;
}tuya_ble_dp_data_reported_t;    


typedef struct{
    uint32_t timestamp;
	uint8_t *p_data;
	uint16_t data_len;
}tuya_ble_dp_data_with_time_reported_t;   


typedef struct{
    uint8_t time_string[13+1];
	uint8_t *p_data;
	uint16_t data_len;
}tuya_ble_dp_data_with_time_string_reported_t;  


typedef struct{
	uint8_t reserve;
}tuya_ble_factory_reset_t;  

/*   
 * ota data
 * */
 
typedef enum 
{
	TUYA_BLE_OTA_REQ,
	TUYA_BLE_OTA_FILE_INFO,
	TUYA_BLE_OTA_FILE_OFFSET_REQ,
	TUYA_BLE_OTA_DATA,
    TUYA_BLE_OTA_END,
    TUYA_BLE_OTA_UNKONWN,
}tuya_ble_ota_data_type_t; 


typedef struct{
	tuya_ble_ota_data_type_t type;	
	uint16_t data_len;
	uint8_t *p_data;
}tuya_ble_ota_response_t;



typedef struct{
	uint16_t data_len;
	uint8_t *p_data;
}tuya_ble_passthrough_data_t;


typedef struct{
    uint8_t channel;
	uint16_t data_len;
	uint8_t *p_data;
}tuya_ble_production_test_response_data_t;

typedef struct{
    uint32_t cmd;
	uint16_t data_len;
	uint8_t *p_data;
}tuya_ble_uart_cmd_t;


typedef struct{
    uint32_t cmd;
	uint16_t data_len;
	uint8_t *p_data;
}tuya_ble_ble_cmd_t;

typedef struct{
	int16_t result_code;
}tuya_ble_net_config_response_t;


typedef struct{
	uint8_t cmd;
}tuya_ble_connecting_request_data_t;


typedef struct{
  int32_t evt_id;  
  void *data;
  void (*custom_event_handler)(int32_t evt_id,void*data);
} tuya_ble_custom_evt_t;


typedef enum{
    TUYA_BLE_CONNECTED,
    TUYA_BLE_DISCONNECTED,
}tuya_ble_connect_status_change_t;


typedef struct{
	uint8_t result_code;
}tuya_ble_ubound_response_t;


typedef struct{
	uint8_t result_code;
}tuya_ble_anomaly_ubound_response_t;


typedef struct{
	uint8_t result_code;
}tuya_ble_device_reset_response_t;

typedef struct{
	uint8_t time_type;  //0-13-byte millisecond string ,1 - normal time format
}tuya_ble_time_req_data_t;

typedef struct{
	uint8_t type;  //0-pair , 1-unbound req ,2- anomaly_unbound, 3-device reset
    tuya_ble_status_t status;
    void *p_param;
}tuya_ble_save_sys_settings_callback_data_t;

typedef struct{
  tuya_ble_evt_t  event;
  void (*event_handler)(void*evt);
} tuya_ble_evt_hdr_t;


/*
 * tuya ble sdk evt parameters union
 * */
typedef struct {
	tuya_ble_evt_hdr_t hdr;
	union {
		tuya_ble_mtu_data_receive_t  mtu_data;
		tuya_ble_device_info_data_t  device_info_data;
		tuya_ble_dp_data_reported_t  reported_data;
		tuya_ble_dp_data_with_time_reported_t       reported_with_time_data;
        tuya_ble_dp_data_with_time_string_reported_t reported_with_time_string_data;
		tuya_ble_factory_reset_t   factory_reset_data;
		tuya_ble_ota_response_t ota_response_data;
		tuya_ble_passthrough_data_t passthrough_data;
        tuya_ble_production_test_response_data_t prod_test_res_data;
		tuya_ble_uart_cmd_t uart_cmd_data;
        tuya_ble_ble_cmd_t    ble_cmd_data;
        tuya_ble_net_config_response_t net_config_response_data;
        tuya_ble_custom_evt_t custom_evt;
        tuya_ble_connect_status_change_t connect_change_evt;
        tuya_ble_ubound_response_t ubound_res_data;
        tuya_ble_anomaly_ubound_response_t anomaly_ubound_res_data;
        tuya_ble_device_reset_response_t device_reset_res_data;
        tuya_ble_time_req_data_t  time_req_data;
        tuya_ble_connecting_request_data_t connecting_request_data;
        tuya_ble_save_sys_settings_callback_data_t sys_settings_save_callback_data;
	};
} tuya_ble_evt_param_t;




/*
 * tuya ble call back event type.
 * */
typedef enum {
    TUYA_BLE_CB_EVT_CONNECTE_STATUS = TUYA_BLE_CB_EVT_BASE, 
    TUYA_BLE_CB_EVT_DP_WRITE, 
	TUYA_BLE_CB_EVT_DP_QUERY,
	TUYA_BLE_CB_EVT_OTA_DATA,
	TUYA_BLE_CB_EVT_NETWORK_INFO,
	TUYA_BLE_CB_EVT_WIFI_SSID,
	TUYA_BLE_CB_EVT_TIME_STAMP,
	TUYA_BLE_CB_EVT_TIME_NORMAL,
    TUYA_BLE_CB_EVT_DATA_PASSTHROUGH,
    TUYA_BLE_CB_EVT_DP_DATA_REPORT_RESPONSE,
    TUYA_BLE_CB_EVT_DP_DATA_WTTH_TIME_REPORT_RESPONSE,
    TUYA_BLE_CB_EVT_UNBOUND,
    TUYA_BLE_CB_EVT_ANOMALY_UNBOUND,
    TUYA_BLE_CB_EVT_DEVICE_RESET,
    TUYA_BLE_CB_EVT_UPDATE_LOGIN_KEY_VID,
} tuya_ble_cb_evt_t;

/*   
 * current connect status
 *@note 
 * */
typedef enum{
    UNBONDING_UNCONN = 0,   
    UNBONDING_CONN,
    BONDING_UNCONN,
    BONDING_CONN,
    BONDING_UNAUTH_CONN,  
    UNBONDING_UNAUTH_CONN,
    UNKNOW_STATUS
}tuya_ble_connect_status_t;

/*
 * dp data  buffer:  (Dp_id,Dp_type,Dp_len,Dp_data),(Dp_id,Dp_type,Dp_len,Dp_data),....
 * */
typedef struct{
	uint8_t *p_data;
	uint16_t data_len;
}tuya_ble_dp_write_data_t;

/*
 * query dp point data,if data_len is 0,means query all dp point data,otherwise query the dp point in p_data buffer.
 * */
typedef struct{
	uint8_t *p_data;
	uint16_t data_len;
}tuya_ble_dp_query_data_t;

 
typedef struct{
	tuya_ble_ota_data_type_t type;	
	uint16_t data_len;
	uint8_t *p_data;
}tuya_ble_ota_data_t;

/*
 * network data,unformatted json data,for example " {"wifi_ssid":"tuya","password":"12345678","token":"xxxxxxxxxx"} "
 * */
typedef struct{
	uint16_t data_len;//include '\0'
	uint8_t *p_data;
}tuya_ble_network_data_t;

/*
 * wifi ssid data,unformatted json data,for example " {"wifi_ssid":"tuya","password":"12345678"} "
 * */
typedef struct{
	uint16_t data_len;//include '\0'
	uint8_t *p_data;
}tuya_ble_wifi_ssid_data_t;


/*
 * uninx timestamp 
 * */
typedef struct{
	uint8_t timestamp_string[14];
	int16_t  time_zone;   //actual time zone Multiply by 100.
}tuya_ble_timestamp_data_t;


/*
 * normal time formatted
 * */
typedef struct{
	uint16_t nYear;
    uint8_t nMonth;
    uint8_t nDay;
    uint8_t nHour;
    uint8_t nMin;
    uint8_t nSec;
    uint8_t DayIndex; /* 0 = Sunday */
	int16_t time_zone;   //actual time zone Multiply by 100.
}tuya_ble_time_noraml_data_t;


/*
 * 
 * */
typedef struct{
	uint8_t status;
}tuya_ble_dp_data_report_response_t;

/*
 * 
 * */
typedef struct{
	uint8_t status;
}tuya_ble_dp_data_with_time_report_response_t;


/*
 * 
 * */
typedef struct{
	uint8_t data;
}tuya_ble_unbound_data_t;

/*
 * 
 * */
typedef struct{
	uint8_t data;
}tuya_ble_anomaly_unbound_data_t;

/*
 * 
 * */
typedef struct{
	uint8_t data;
}tuya_ble_device_reset_data_t;


/*
 * network data,unformatted json data,for example " {"wifi_ssid":"tuya","password":"12345678","token":"xxxxxxxxxx"} "
 * */
typedef struct{
    uint8_t login_key_len;
    uint8_t vid_len;
	uint8_t login_key[LOGIN_KEY_LEN];
	uint8_t vid[DEVICE_VIRTUAL_ID_LEN];
}tuya_ble_login_key_vid_data_t;



/*
 * tuya ble sdk callback parameters union
 * */
typedef struct {
	tuya_ble_cb_evt_t evt;
	union {
		tuya_ble_connect_status_t connect_status;
		tuya_ble_dp_write_data_t  dp_write_data;
		tuya_ble_dp_query_data_t  dp_query_data;
		tuya_ble_ota_data_t       ota_data;
		tuya_ble_network_data_t   network_data;
		tuya_ble_wifi_ssid_data_t wifi_info_data;
		tuya_ble_timestamp_data_t timestamp_data;
		tuya_ble_time_noraml_data_t time_normal_data;
        tuya_ble_passthrough_data_t ble_passthrough_data;
        tuya_ble_dp_data_report_response_t dp_response_data;
        tuya_ble_dp_data_with_time_report_response_t dp_with_time_response_data;
        tuya_ble_unbound_data_t unbound_data;
        tuya_ble_anomaly_unbound_data_t anomaly_unbound_data;
        tuya_ble_device_reset_data_t device_reset_data;
        tuya_ble_login_key_vid_data_t device_login_key_vid_data;
	};
} tuya_ble_cb_evt_param_t;


/* TIMER related */
typedef void *tuya_ble_timer_t;

typedef void (*tuya_ble_timer_handler_t)(void*);


typedef enum {
    TUYA_BLE_TIMER_SINGLE_SHOT,
    TUYA_BLE_TIMER_REPEATED,
} tuya_ble_timer_mode;



typedef void (*tuya_ble_callback_t)(tuya_ble_cb_evt_param_t* param);


typedef struct
{
      uint32_t  crc;
	  uint32_t  settings_version;
	  uint8_t   h_id[H_ID_LEN];
	  uint8_t   device_id[DEVICE_ID_LEN];
	  uint8_t   mac[MAC_LEN];
	  uint8_t   auth_key[AUTH_KEY_LEN];
      uint8_t   mac_string[MAC_LEN*2];
	  uint8_t  res[128];
}tuya_ble_auth_settings_t;   


typedef struct
{
      uint32_t  crc;
	  uint32_t  settings_version;
      tuya_ble_product_id_type_t pid_type;
      uint8_t   pid_len;
	  uint8_t   common_pid[TUYA_BLE_PRODUCT_ID_MAX_LEN];
	  uint8_t   login_key[LOGIN_KEY_LEN];
      uint8_t   ecc_secret_key[ECC_SECRET_KEY_LEN];
      uint8_t   device_virtual_id[DEVICE_VIRTUAL_ID_LEN];
      uint8_t   user_rand[PAIR_RANDOM_LEN];
	  uint8_t   bound_flag;
      uint8_t   factory_test_flag;
      uint8_t  res[127];
}tuya_ble_sys_settings_t; 


typedef struct
{
	tuya_ble_auth_settings_t auth_settings;
	tuya_ble_sys_settings_t  sys_settings;
    tuya_ble_product_id_type_t pid_type;
    uint8_t pid_len;
	uint8_t pid[TUYA_BLE_PRODUCT_ID_MAX_LEN];
}tuya_ble_parameters_settings_t;


typedef struct
{
      tuya_ble_product_id_type_t pid_type;
      uint8_t   pid_len;
	  uint8_t   pid[TUYA_BLE_PRODUCT_ID_MAX_LEN];
	  uint8_t   h_id[H_ID_LEN];
	  uint8_t   device_id[DEVICE_ID_LEN];
	  uint8_t   mac[MAC_LEN];
	  uint8_t   auth_key[AUTH_KEY_LEN];
}tuya_ble_factory_id_data_t;  


#endif
