/**
 * \file tuya_ble_storage.c
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
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_type.h"
#include "tuya_ble_main.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_log.h"


typedef struct
{
    tuya_ble_auth_settings_t flash_settings_auth;
    tuya_ble_auth_settings_t flash_settings_auth_backup;
} tuya_ble_storage_auth_settings_t;

typedef struct
{
    tuya_ble_sys_settings_t flash_settings_sys;
    tuya_ble_sys_settings_t flash_settings_sys_backup;
} tuya_ble_storage_sys_settings_t;


static tuya_ble_nv_async_callback_t storage_init_complete = NULL;
static tuya_ble_storage_auth_settings_t read_storage_settings_auth;
static tuya_ble_storage_sys_settings_t read_storage_settings_sys;
static uint8_t tuya_ble_storage_init_flag = 0;

static tuya_ble_nv_async_callback_t storage_save_auth_settings_completed = NULL;
static uint8_t tuya_ble_storage_save_auth_settings_flag = 0;

static tuya_ble_nv_async_callback_t storage_save_sys_settings_completed = NULL;
static uint8_t tuya_ble_storage_save_sys_settings_flag = 0;


static uint16_t save_sys_settings_sn = 0;
static uint16_t save_auth_settings_sn = 0;


/*********************************************************************************************************************************************
tuya_ble_storage_save_sys_settings_async:
**********************************************************************************************************************************************/

static void tuya_ble_storage_save_sys_settings_completed(void * p_param,tuya_ble_status_t result)
{
    uint8_t *p_data = NULL;
    uint16_t current_sn = 0;
    uint16_t data_len = 0;

    if(p_param)
    {
        p_data = (uint8_t *)p_param;
        current_sn = (p_data[0]<<8)+p_data[1];
        data_len = (p_data[2]<<8)+p_data[3];
        if (result == TUYA_BLE_SUCCESS)
        {
            TUYA_BLE_LOG_INFO("save flash_settings_sys data succeed, current sn = %d ,data addr = 0x%08x.",current_sn,p_data);
        }
        else
        {
            TUYA_BLE_LOG_ERROR("save flash_settings_sys data failed, err code = %d ,current sn = %d ,data addr = 0x%08x.",result,current_sn,p_data);
        }

        if(storage_save_sys_settings_completed)
            storage_save_sys_settings_completed(NULL,result);//FAIL
        tuya_ble_free(p_data);
        tuya_ble_storage_save_sys_settings_flag = 0;
        storage_save_sys_settings_completed = NULL;
    }
    else
    {
        TUYA_BLE_LOG_ERROR("save flash_settings_sys data was failed due to internal error;");  //It should never happen
        if(storage_save_sys_settings_completed)
            storage_save_sys_settings_completed(NULL,TUYA_BLE_ERR_INTERNAL);//error
        tuya_ble_storage_save_sys_settings_flag = 0;
        storage_save_sys_settings_completed = NULL;
    }

}

static void tuya_ble_storage_save_sys_settings_async_backup_erase_completed(void * p_param,tuya_ble_status_t result)
{
    uint8_t *p_data = NULL;
    uint16_t data_len = 0;

    if(p_param)
    {
        p_data = (uint8_t *)p_param;
        data_len = (p_data[2]<<8)+p_data[3];
    }
    else
    {
        if(storage_save_sys_settings_completed)
            storage_save_sys_settings_completed(NULL,TUYA_BLE_ERR_INTERNAL);//error
        tuya_ble_storage_save_sys_settings_flag = 0;
        storage_save_sys_settings_completed = NULL;
        return;
    }

    if (result == TUYA_BLE_SUCCESS)
    {
        tuya_ble_nv_write_async(TUYA_BLE_SYS_FLASH_BACKUP_ADDR,p_data+8,data_len,p_data,tuya_ble_storage_save_sys_settings_completed);
        TUYA_BLE_LOG_DEBUG("save flash_settings_sys data start write data to TUYA_BLE_SYS_FLASH_BACKUP_ADDR, current sn = %d ,data addr = 0x%08x.",((p_data[0]<<8)+p_data[1]),p_data);
    }
    else
    {
        if(storage_save_sys_settings_completed)
            storage_save_sys_settings_completed(NULL,result);//FAIL
        tuya_ble_free(p_data);
        tuya_ble_storage_save_sys_settings_flag = 0;
        storage_save_sys_settings_completed = NULL;
    }

}

static void tuya_ble_storage_save_sys_settings_async_write_completed(void * p_param,tuya_ble_status_t result)
{
    uint8_t *p_data = (uint8_t *)p_param;

    if(p_param==NULL)
    {
        if(storage_save_sys_settings_completed)
            storage_save_sys_settings_completed(NULL,TUYA_BLE_ERR_INTERNAL);//error
        tuya_ble_storage_save_sys_settings_flag = 0;
        storage_save_sys_settings_completed = NULL;
        return;
    }

    if (result == TUYA_BLE_SUCCESS)
    {
        tuya_ble_nv_erase_async(TUYA_BLE_SYS_FLASH_BACKUP_ADDR,TUYA_NV_ERASE_MIN_SIZE, p_param,tuya_ble_storage_save_sys_settings_async_backup_erase_completed);

        TUYA_BLE_LOG_DEBUG("save flash_settings_sys data start erase data to TUYA_BLE_SYS_FLASH_BACKUP_ADDR, current sn = %d ,data addr = 0x%08x.",((p_data[0]<<8)+p_data[1]),p_data);
    }
    else
    {
        if(storage_save_sys_settings_completed)
            storage_save_sys_settings_completed(NULL,result);//FAIL
        tuya_ble_free(p_data);
        tuya_ble_storage_save_sys_settings_flag = 0;
        storage_save_sys_settings_completed = NULL;
    }

}


static void tuya_ble_storage_save_sys_settings_async_erase_completed(void * p_param,tuya_ble_status_t result)
{
    uint8_t *p_data = NULL;
    uint16_t data_len = 0;

    if(p_param)
    {
        p_data = (uint8_t *)p_param;
        data_len = (p_data[2]<<8)+p_data[3];
    }
    else
    {
        if(storage_save_sys_settings_completed)
            storage_save_sys_settings_completed(NULL,TUYA_BLE_ERR_INTERNAL);//error
        tuya_ble_storage_save_sys_settings_flag = 0;
        storage_save_sys_settings_completed = NULL;
        return;
    }

    if (result == TUYA_BLE_SUCCESS)
    {
        tuya_ble_nv_write_async(TUYA_BLE_SYS_FLASH_ADDR,p_data+8,data_len,p_data,tuya_ble_storage_save_sys_settings_async_write_completed);
        TUYA_BLE_LOG_DEBUG("save flash_settings_sys data start write data to TUYA_BLE_SYS_FLASH_ADDR,current sn = %d ,data addr = 0x%08x.",((p_data[0]<<8)+p_data[1]),p_data);
    }
    else
    {
        if(storage_save_sys_settings_completed)
            storage_save_sys_settings_completed(NULL,result);//FAIL
        tuya_ble_free(p_data);
        tuya_ble_storage_save_sys_settings_flag = 0;
        storage_save_sys_settings_completed = NULL;
    }

}

void tuya_ble_storage_save_sys_settings_async(tuya_ble_nv_async_callback_t callback)
{
    uint8_t *p_data = NULL;
    uint16_t data_len = 0;

    if(tuya_ble_storage_save_sys_settings_flag)
    {
        if(callback)
        {
            callback(NULL,TUYA_BLE_ERR_BUSY);
        }
        return;
    }
    else
    {
        tuya_ble_storage_save_sys_settings_flag = 1;
    }

    storage_save_sys_settings_completed = callback;

    tuya_ble_current_para.sys_settings.crc = tuya_ble_crc32_compute((uint8_t *)&tuya_ble_current_para.sys_settings+4,sizeof(tuya_ble_sys_settings_t)-4,NULL);

    data_len = sizeof(tuya_ble_sys_settings_t);

    p_data = (uint8_t *)tuya_ble_malloc(data_len+8);

    if(p_data == NULL)
    {
        TUYA_BLE_LOG_ERROR("save flash_settings_auth data malloc failed.");
        if(storage_save_sys_settings_completed)
            storage_save_sys_settings_completed(NULL,TUYA_BLE_ERR_NO_MEM);
        tuya_ble_storage_save_auth_settings_flag = 0;
        storage_save_sys_settings_completed = NULL;
    }
    else
    {
        tuya_ble_device_enter_critical();
        save_sys_settings_sn++;
        if(save_sys_settings_sn>=0xFFFF)
            save_sys_settings_sn = 0;
        p_data[0] = save_sys_settings_sn>>8;
        p_data[1] = save_sys_settings_sn;
        p_data[2] = data_len>>8;
        p_data[3] = data_len;
        p_data[4] = 0;
        p_data[5] = 0;
        p_data[6] = 0;
        p_data[7] = 0;
        memcpy(p_data+8,(uint8_t *)&tuya_ble_current_para.sys_settings,data_len);
        tuya_ble_nv_erase_async(TUYA_BLE_SYS_FLASH_ADDR,TUYA_NV_ERASE_MIN_SIZE, p_data,tuya_ble_storage_save_sys_settings_async_erase_completed);
        tuya_ble_device_exit_critical();

        TUYA_BLE_LOG_DEBUG("save flash_settings_sys data start erase data to TUYA_BLE_SYS_FLASH_ADDR, current sn = %d ,data addr = 0x%08x.",((p_data[0]<<8)+p_data[1]),p_data);

    }

}


uint8_t tuya_ble_get_storage_save_sys_settings_flag(void)
{
    return tuya_ble_storage_save_sys_settings_flag;
}


/*********************************************************************************************************************************************
tuya_ble_storage_save_auth_settings_async:
**********************************************************************************************************************************************/

static void tuya_ble_storage_save_auth_settings_completed(void * p_param,tuya_ble_status_t result)
{
    uint8_t *p_data = NULL;
    uint16_t current_sn = 0;
    uint16_t data_len = 0;

    if(p_param)
    {
        p_data = (uint8_t *)p_param;
        current_sn = (p_data[0]<<8)+p_data[1];
        data_len = (p_data[2]<<8)+p_data[3];

        if (result == TUYA_BLE_SUCCESS)
        {
            TUYA_BLE_LOG_INFO("save flash_settings_auth data succeed, current sn = %d ,data addr = 0x%08x.",current_sn,p_data);
        }
        else
        {
            TUYA_BLE_LOG_ERROR("save flash_settings_auth data failed, err code = %d ,current sn = %d ,data addr = 0x%08x.",result,current_sn,p_data);
        }
        if(storage_save_auth_settings_completed)
            storage_save_auth_settings_completed(NULL,result);
        tuya_ble_free(p_data);
        tuya_ble_storage_save_auth_settings_flag = 0;
        storage_save_auth_settings_completed = NULL;
    }
    else
    {
        TUYA_BLE_LOG_ERROR("save flash_settings_auth data was failed due to internal error;");  //It should never happen
        if(storage_save_auth_settings_completed)
            storage_save_auth_settings_completed(NULL,TUYA_BLE_ERR_INTERNAL);
        tuya_ble_storage_save_auth_settings_flag = 0;
        storage_save_auth_settings_completed = NULL;
    }

}

static void tuya_ble_storage_save_auth_settings_async_backup_erase_completed(void * p_param,tuya_ble_status_t result)
{
    uint8_t *p_data = NULL;
    uint16_t data_len = 0;
    tuya_ble_status_t err_code = TUYA_BLE_SUCCESS;

    if(p_param)
    {
        p_data = (uint8_t *)p_param;
        data_len = (p_data[2]<<8)+p_data[3];
    }
    else
    {
        if(storage_save_auth_settings_completed)
            storage_save_auth_settings_completed(NULL,TUYA_BLE_ERR_INTERNAL);//error
        tuya_ble_storage_save_auth_settings_flag = 0;
        storage_save_auth_settings_completed = NULL;
        return;
    }

    if (result == TUYA_BLE_SUCCESS)
    {
        tuya_ble_nv_write_async(TUYA_BLE_AUTH_FLASH_BACKUP_ADDR,p_data+8,data_len,p_data,tuya_ble_storage_save_auth_settings_completed);
        TUYA_BLE_LOG_DEBUG("save flash_settings_auth data start write data to TUYA_BLE_AUTH_FLASH_BACKUP_ADDR,current sn = %d ,data addr = 0x%08x.",((p_data[0]<<8)+p_data[1]),p_data);
    }
    else
    {
        if(storage_save_auth_settings_completed)
            storage_save_auth_settings_completed(NULL,result);//FAIL
        tuya_ble_free(p_data);
        tuya_ble_storage_save_auth_settings_flag = 0;
        storage_save_auth_settings_completed = NULL;
    }

}

static void tuya_ble_storage_save_auth_settings_async_write_completed(void * p_param,tuya_ble_status_t result)
{
    uint8_t *p_data = NULL;

    if(p_param==NULL)
    {
        if(storage_save_auth_settings_completed)
            storage_save_auth_settings_completed(NULL,TUYA_BLE_ERR_INTERNAL);//error
        tuya_ble_storage_save_auth_settings_flag = 0;
        storage_save_auth_settings_completed = NULL;
        return;
    }

    if (result == TUYA_BLE_SUCCESS)
    {
        tuya_ble_nv_erase_async(TUYA_BLE_AUTH_FLASH_BACKUP_ADDR,TUYA_NV_ERASE_MIN_SIZE, p_param,tuya_ble_storage_save_auth_settings_async_backup_erase_completed);
        p_data = (uint8_t *)p_param;
        TUYA_BLE_LOG_DEBUG("save flash_settings_auth data start erase data to TUYA_BLE_AUTH_FLASH_BACKUP_ADDR, current sn = %d ,data addr = 0x%08x.",((p_data[0]<<8)+p_data[1]),p_data);
    }
    else
    {
        if(storage_save_auth_settings_completed)
            storage_save_auth_settings_completed(NULL,result);//FAIL
        tuya_ble_free(p_data);
        tuya_ble_storage_save_auth_settings_flag = 0;
        storage_save_auth_settings_completed = NULL;
    }

}


static void tuya_ble_storage_save_auth_settings_async_erase_completed(void * p_param,tuya_ble_status_t result)
{
    uint8_t *p_data = NULL;
    uint16_t data_len = 0;

    if(p_param)
    {
        p_data = (uint8_t *)p_param;
        data_len = (p_data[2]<<8)+p_data[3];
    }
    else
    {
        if(storage_save_auth_settings_completed)
            storage_save_auth_settings_completed(NULL,TUYA_BLE_ERR_INTERNAL);//error
        tuya_ble_storage_save_auth_settings_flag = 0;
        storage_save_auth_settings_completed = NULL;
        return;
    }

    if (result == TUYA_BLE_SUCCESS)
    {
        tuya_ble_nv_write_async(TUYA_BLE_AUTH_FLASH_ADDR,p_data+8,data_len,p_data,tuya_ble_storage_save_auth_settings_async_write_completed);
        TUYA_BLE_LOG_DEBUG("save flash_settings_auth data start write data to TUYA_BLE_AUTH_FLASH_ADDR, current sn = %d , data addr = 0x%08x.",((p_data[0]<<8)+p_data[1]),p_data);

    }
    else
    {
        if(storage_save_auth_settings_completed)
            storage_save_auth_settings_completed(NULL,result);//FAIL
        tuya_ble_free(p_data);
        tuya_ble_storage_save_auth_settings_flag = 0;
        storage_save_auth_settings_completed = NULL;
    }

}

void tuya_ble_storage_save_auth_settings_async(tuya_ble_nv_async_callback_t callback)
{
    uint8_t *p_data = NULL;
    uint16_t data_len = 0;
    tuya_ble_status_t err_code = TUYA_BLE_SUCCESS;

    if(tuya_ble_storage_save_auth_settings_flag)
    {
        if(callback)
            callback(NULL,TUYA_BLE_ERR_BUSY);
        return;
    }
    else
    {
        tuya_ble_storage_save_auth_settings_flag = 1;
    }

    storage_save_auth_settings_completed = callback;

    tuya_ble_current_para.auth_settings.crc = tuya_ble_crc32_compute((uint8_t *)&tuya_ble_current_para.auth_settings+4,sizeof(tuya_ble_current_para.auth_settings)-4,NULL);

    data_len = sizeof(tuya_ble_auth_settings_t);

    p_data = (uint8_t *)tuya_ble_malloc(data_len+8);

    if(p_data == NULL)
    {
        TUYA_BLE_LOG_ERROR("save flash_settings_auth data malloc failed.");
        if(storage_save_auth_settings_completed)
            storage_save_auth_settings_completed(NULL,TUYA_BLE_ERR_NO_MEM);
        tuya_ble_storage_save_auth_settings_flag = 0;
        storage_save_auth_settings_completed = NULL;
    }
    else
    {
        tuya_ble_device_enter_critical();
        save_auth_settings_sn++;
        if(save_auth_settings_sn>=0xFFFF)
            save_auth_settings_sn = 0;
        p_data[0] = save_auth_settings_sn>>8;
        p_data[1] = save_auth_settings_sn;
        p_data[2] = data_len>>8;
        p_data[3] = data_len;
        p_data[4] = 0;
        p_data[5] = 0;
        p_data[6] = 0;
        p_data[7] = 0;
        memcpy(p_data+8,(uint8_t *)&tuya_ble_current_para.auth_settings,data_len);
        tuya_ble_nv_erase_async(TUYA_BLE_AUTH_FLASH_ADDR,TUYA_NV_ERASE_MIN_SIZE, p_data,tuya_ble_storage_save_auth_settings_async_erase_completed);
        tuya_ble_device_exit_critical();

        TUYA_BLE_LOG_DEBUG("save flash_settings_auth data start erase data to TUYA_BLE_AUTH_FLASH_ADDR, current sn = %d ,data addr = 0x%08x.",((p_data[0]<<8)+p_data[1]),p_data);

    }

}

/*********************************************************************************************************************************************

**********************************************************************************************************************************************/

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



static uint32_t auth_settings_crc_get(tuya_ble_auth_settings_t const * p_settings)
{
    // The crc is calculated from the s_dfu_settings struct, except the crc itself and the init command
    return tuya_ble_crc32_compute((uint8_t*)(p_settings) + 4, sizeof(tuya_ble_auth_settings_t) - 4, NULL);
}

static bool auth_settings_crc_ok(tuya_ble_auth_settings_t const * p_settings)
{
    uint32_t crc;
    if (p_settings->crc != 0xFFFFFFFF)
    {
        // CRC is set. Content must be valid
        crc = auth_settings_crc_get(p_settings);
        if (crc == p_settings->crc)
        {
            return true;
        }
    }
    return false;
}


static uint32_t sys_settings_crc_get(tuya_ble_sys_settings_t const * p_settings)
{
    // The crc is calculated from the s_dfu_settings struct, except the crc itself and the init command
    return tuya_ble_crc32_compute((uint8_t*)(p_settings) + 4, sizeof(tuya_ble_sys_settings_t) - 4, NULL);
}



static bool sys_settings_crc_ok(tuya_ble_sys_settings_t  const* p_settings)
{
    uint32_t crc;
    if (p_settings->crc != 0xFFFFFFFF)
    {
        // CRC is set. Content must be valid
        crc = sys_settings_crc_get(p_settings);
        if (crc == p_settings->crc)
        {
            return true;
        }
    }
    return false;
}


/*********************************************************************************************************************************************
tuya_ble_storage_init_async:
**********************************************************************************************************************************************/

static void tuya_ble_storage_init_async_read_settings_sys_backup_completed(void * p_param,tuya_ble_status_t result)
{
    bool settings_valid ;
    bool settings_backup_valid;

    if (result == TUYA_BLE_SUCCESS)
    {
        settings_valid = sys_settings_crc_ok(&read_storage_settings_sys.flash_settings_sys);
        settings_backup_valid = sys_settings_crc_ok(&read_storage_settings_sys.flash_settings_sys_backup);

        if(settings_valid)
        {
            memcpy(&tuya_ble_current_para.sys_settings,&read_storage_settings_sys.flash_settings_sys,sizeof(tuya_ble_sys_settings_t));
        }
        else if(settings_backup_valid)
        {
            memcpy(&tuya_ble_current_para.sys_settings,&read_storage_settings_sys.flash_settings_sys_backup,sizeof(tuya_ble_sys_settings_t));
        }
        else
        {
            memset(&tuya_ble_current_para.sys_settings,0,sizeof(tuya_ble_sys_settings_t));
            tuya_ble_current_para.sys_settings.factory_test_flag = 0xFF;
        }

        storage_init_complete(p_param,TUYA_BLE_SUCCESS);
        tuya_ble_storage_init_flag = 0;
    }
    else
    {
        storage_init_complete(p_param,TUYA_BLE_ERR_INTERNAL);
        tuya_ble_storage_init_flag = 0;
    }
}

static void tuya_ble_storage_init_async_read_settings_sys_completed(void * p_param,tuya_ble_status_t result)
{

    if (result == TUYA_BLE_SUCCESS)
    {
        tuya_ble_nv_read_async(TUYA_BLE_SYS_FLASH_BACKUP_ADDR,(uint8_t *)&read_storage_settings_sys.flash_settings_sys_backup,sizeof(tuya_ble_sys_settings_t)
                               ,p_param,tuya_ble_storage_init_async_read_settings_sys_backup_completed);
    }
    else
    {
        storage_init_complete(p_param,TUYA_BLE_ERR_INTERNAL);
        tuya_ble_storage_init_flag = 0;
    }
}

static void tuya_ble_storage_init_async_read_settings_auth_backup_completed(void * p_param,tuya_ble_status_t result)
{
    bool settings_valid ;
    bool settings_backup_valid;

    if (result == TUYA_BLE_SUCCESS)
    {
        settings_valid = auth_settings_crc_ok(&read_storage_settings_auth.flash_settings_auth);
        settings_backup_valid = auth_settings_crc_ok(&read_storage_settings_auth.flash_settings_auth_backup);

        if(settings_valid)
        {
            memcpy(&tuya_ble_current_para.auth_settings,&read_storage_settings_auth.flash_settings_auth,sizeof(tuya_ble_auth_settings_t));
        }
        else if(settings_backup_valid)
        {
            memcpy(&tuya_ble_current_para.auth_settings,&read_storage_settings_auth.flash_settings_auth_backup,sizeof(tuya_ble_auth_settings_t));
        }
        else
        {
            memset(&tuya_ble_current_para.auth_settings,0,sizeof(tuya_ble_auth_settings_t));
        }

        tuya_ble_nv_read_async(TUYA_BLE_SYS_FLASH_ADDR,(uint8_t *)&read_storage_settings_sys.flash_settings_sys,sizeof(tuya_ble_sys_settings_t)
                               ,p_param,tuya_ble_storage_init_async_read_settings_sys_completed);
    }
    else
    {
        storage_init_complete(p_param,TUYA_BLE_ERR_INTERNAL);
        tuya_ble_storage_init_flag = 0;
    }
}

static void tuya_ble_storage_init_async_read_settings_auth_completed(void * p_param,tuya_ble_status_t result)
{
    if (result == TUYA_BLE_SUCCESS)
    {
        tuya_ble_nv_read_async(TUYA_BLE_AUTH_FLASH_BACKUP_ADDR,(uint8_t *)&read_storage_settings_auth.flash_settings_auth_backup,sizeof(tuya_ble_auth_settings_t)
                               ,p_param,tuya_ble_storage_init_async_read_settings_auth_backup_completed);
    }
    else
    {
        storage_init_complete(p_param,TUYA_BLE_ERR_INTERNAL);
        tuya_ble_storage_init_flag = 0;
    }
}


void tuya_ble_storage_init_async(void *p_param,tuya_ble_nv_async_callback_t callback)
{
    if(tuya_ble_storage_init_flag)
    {
        return;
    }
    else
    {
        tuya_ble_storage_init_flag = 1;
    }

    storage_init_complete = callback;
    tuya_ble_nv_init();

    memset(&read_storage_settings_auth,0,sizeof(tuya_ble_storage_auth_settings_t));
    tuya_ble_nv_read_async(TUYA_BLE_AUTH_FLASH_ADDR,(uint8_t *)&read_storage_settings_auth.flash_settings_auth,sizeof(tuya_ble_auth_settings_t)
                           ,p_param,tuya_ble_storage_init_async_read_settings_auth_completed);

}



#if (TUYA_BLE_DEVICE_AUTH_DATA_STORE)
/**
 * @brief   Function for write pid to nv
 *
 * @note
 *
 * */
void tuya_ble_storage_write_pid_async(tuya_ble_product_id_type_t pid_type,uint8_t pid_len,uint8_t *pid,tuya_ble_nv_async_callback_t callback)
{
    uint8_t is_write = 0;

    if(pid_len>TUYA_BLE_PRODUCT_ID_MAX_LEN)
    {
        if(callback)
            callback(NULL,TUYA_BLE_ERR_INVALID_PARAM);
        return;
    }

    if((pid_type!=tuya_ble_current_para.pid_type)||(pid_len!=tuya_ble_current_para.pid_len))
    {
        tuya_ble_current_para.pid_type = pid_type;
        tuya_ble_current_para.pid_len = pid_len;
        memcpy(tuya_ble_current_para.pid,pid,pid_len);
        tuya_ble_current_para.sys_settings.pid_type = pid_type;
        tuya_ble_current_para.sys_settings.pid_len = pid_len;
        memcpy(tuya_ble_current_para.sys_settings.common_pid,pid,pid_len);
        is_write = 1;
    }
    else if(memcmp(pid,tuya_ble_current_para.pid,pid_len)!=0)
    {
        tuya_ble_current_para.pid_type = pid_type;
        tuya_ble_current_para.pid_len = pid_len;
        memcpy(tuya_ble_current_para.pid,pid,pid_len);
        tuya_ble_current_para.sys_settings.pid_type = pid_type;
        tuya_ble_current_para.sys_settings.pid_len = pid_len;
        memcpy(tuya_ble_current_para.sys_settings.common_pid,pid,pid_len);
        is_write = 1;
    }
    else
    {

    }
    if(is_write==1)
    {
        tuya_ble_storage_save_sys_settings_async(callback);
    }
    else
    {
        if(callback)
            callback(NULL,TUYA_BLE_SUCCESS);
    }

}


/**
 * @brief   Function for write hid to nv
 *
 * @note
 *
 * */
void tuya_ble_storage_write_hid_async(uint8_t *hid,uint8_t len,tuya_ble_nv_async_callback_t callback)
{

    if(len!=H_ID_LEN)
    {
        if(callback)
            callback(NULL,TUYA_BLE_ERR_INVALID_PARAM);
        return;
    }
    else
    {
        if(memcmp(hid,tuya_ble_current_para.auth_settings.h_id,H_ID_LEN)!=0)
        {
            memcpy(tuya_ble_current_para.auth_settings.h_id,hid,H_ID_LEN);
            tuya_ble_storage_save_auth_settings_async(callback);
        }
        else
        {
            if(callback)
                callback(NULL,TUYA_BLE_SUCCESS);
        }
    }

}

/**
 * @brief   Function for read id parameters
 *
 * @note
 *
 * */
tuya_ble_status_t tuya_ble_storage_read_id_info(tuya_ble_factory_id_data_t *id)
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;

    id->pid_type = tuya_ble_current_para.pid_type;
    id->pid_len = tuya_ble_current_para.pid_len;
    memcpy(id->pid,tuya_ble_current_para.pid,tuya_ble_current_para.pid_len);

    memcpy(id->h_id,tuya_ble_current_para.auth_settings.h_id,H_ID_LEN);
    memcpy(id->device_id,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN);
    memcpy(id->mac,tuya_ble_current_para.auth_settings.mac,MAC_LEN);
    memcpy(id->auth_key,tuya_ble_current_para.auth_settings.auth_key,AUTH_KEY_LEN);

    return ret;
}


static tuya_ble_nv_async_callback_t tuya_ble_storage_write_auth_key_device_id_mac_async_callback = NULL;



static void tuya_ble_storage_write_auth_key_device_id_mac_async_update_sys_completed(void *p_param,tuya_ble_status_t result)
{
    if(result==TUYA_BLE_SUCCESS)
    {
        tuya_ble_adv_change();
        tuya_ble_connect_status_set(UNBONDING_UNCONN);
        TUYA_BLE_LOG_INFO("The state has changed, current bound flag = %d",tuya_ble_current_para.sys_settings.bound_flag);
    }
    if(tuya_ble_storage_write_auth_key_device_id_mac_async_callback)
    {
        tuya_ble_storage_write_auth_key_device_id_mac_async_callback(NULL,result);
        tuya_ble_storage_write_auth_key_device_id_mac_async_callback = NULL;
    }
}


static void tuya_ble_storage_write_auth_key_device_id_mac_async_completed(void *p_param,tuya_ble_status_t result)
{
    if(result==TUYA_BLE_SUCCESS)
    {
        if(tuya_ble_current_para.sys_settings.bound_flag==1)
        {
            memset(tuya_ble_current_para.sys_settings.device_virtual_id,0,DEVICE_VIRTUAL_ID_LEN);
            memset(tuya_ble_current_para.sys_settings.login_key,0,LOGIN_KEY_LEN);
            tuya_ble_current_para.sys_settings.bound_flag= 0;
            tuya_ble_storage_save_sys_settings_async(tuya_ble_storage_write_auth_key_device_id_mac_async_update_sys_completed);

        }
        else
        {
            if(tuya_ble_storage_write_auth_key_device_id_mac_async_callback)
            {
                tuya_ble_storage_write_auth_key_device_id_mac_async_callback(NULL,TUYA_BLE_SUCCESS);
                tuya_ble_storage_write_auth_key_device_id_mac_async_callback = NULL;
            }
        }
    }
    else
    {
        if(tuya_ble_storage_write_auth_key_device_id_mac_async_callback)
        {
            tuya_ble_storage_write_auth_key_device_id_mac_async_callback(NULL,result);
            tuya_ble_storage_write_auth_key_device_id_mac_async_callback = NULL;
        }
    }
}
/**
 * @brief   Function for write auth key/uuid/mac
 *
 * @note  If the id length is 0, the corresponding id will not be written.
 *
 * */
void tuya_ble_storage_write_auth_key_device_id_mac_async(uint8_t *auth_key,uint8_t auth_key_len,uint8_t *device_id,uint8_t device_id_len,
        uint8_t *mac,uint8_t mac_len,uint8_t *mac_string,uint8_t mac_string_len,tuya_ble_nv_async_callback_t callback)
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    uint8_t is_write = 0;

    if(((auth_key_len!=AUTH_KEY_LEN)&&(auth_key_len!=0))||((device_id_len!=DEVICE_ID_LEN)&&(device_id_len!=0))||((mac_len!=MAC_LEN)&&(mac_len!=0)))
    {
        if(callback)
            callback(NULL,TUYA_BLE_ERR_INVALID_PARAM);
        return;
    }
    else
    {
        if(auth_key_len==AUTH_KEY_LEN)
        {
            if(memcmp(tuya_ble_current_para.auth_settings.auth_key,auth_key,AUTH_KEY_LEN)!=0)
            {
                memcpy(tuya_ble_current_para.auth_settings.auth_key,auth_key,AUTH_KEY_LEN);
                is_write = 1;
            }
        }
        if(device_id_len==DEVICE_ID_LEN)
        {
            if(memcmp(tuya_ble_current_para.auth_settings.device_id,device_id,DEVICE_ID_LEN)!=0)
            {
                memcpy(tuya_ble_current_para.auth_settings.device_id,device_id,DEVICE_ID_LEN);
                is_write = 1;
            }
        }
        if(mac_len==MAC_LEN)
        {
            if(memcmp(tuya_ble_current_para.auth_settings.mac,mac,MAC_LEN)!=0)
            {
                memcpy(tuya_ble_current_para.auth_settings.mac,mac,MAC_LEN);
                memcpy(tuya_ble_current_para.auth_settings.mac_string,mac_string,MAC_LEN*2);
                is_write = 1;
            }
        }

        if(is_write==1)
        {
            tuya_ble_storage_write_auth_key_device_id_mac_async_callback = callback;
            tuya_ble_storage_save_auth_settings_async(tuya_ble_storage_write_auth_key_device_id_mac_async_completed);
        }
        else
        {
            if(callback)
                callback(NULL,TUYA_BLE_SUCCESS);
        }

    }

}

#endif










