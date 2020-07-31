/**
 * \file tuya_ble_port.c
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

#include "tuya_ble_port.h"
#include "tuya_ble_type.h"

/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_gap_advertising_adv_data_update(uint8_t const * p_ad_data, uint8_t ad_len)
{
	return TUYA_BLE_SUCCESS;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_gap_advertising_scan_rsp_data_update(uint8_t const *p_sr_data, uint8_t sr_len)
{
	return TUYA_BLE_SUCCESS;
}

/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_gap_disconnect(void)
{
	return TUYA_BLE_SUCCESS;
}

/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_gatt_send_data(const uint8_t *p_data,uint16_t len)
{
	return TUYA_BLE_SUCCESS;
}

 
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */ 
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_timer_create(void** p_timer_id,uint32_t timeout_value_ms, tuya_ble_timer_mode mode,tuya_ble_timer_handler_t timeout_handler)
{
	return TUYA_BLE_SUCCESS;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_timer_delete(void* timer_id)
{
	return TUYA_BLE_SUCCESS;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_timer_start(void* timer_id)
{
	return TUYA_BLE_SUCCESS;
}

/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_timer_restart(void* timer_id,uint32_t timeout_value_ms)
{
	return TUYA_BLE_SUCCESS;
}

/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_timer_stop(void* timer_id)
{
	return TUYA_BLE_SUCCESS;
}


/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK void tuya_ble_device_delay_ms(uint32_t ms)
{
    
}

/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK void tuya_ble_device_delay_us(uint32_t us)
{
    
}


/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_rand_generator(uint8_t* p_buf, uint8_t len)
{
	return TUYA_BLE_SUCCESS;
}
 
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_device_reset(void)
{
	return TUYA_BLE_SUCCESS;
}

/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_gap_addr_get(tuya_ble_gap_addr_t *p_addr)
{
	return TUYA_BLE_SUCCESS;
}

/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_gap_addr_set(tuya_ble_gap_addr_t *p_addr)
{
	return TUYA_BLE_SUCCESS;
}

 /**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_rtc_get_timestamp(uint32_t *timestamp,int32_t *timezone)
{
	return TUYA_BLE_SUCCESS;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_rtc_set_timestamp(uint32_t timestamp,int32_t timezone)
{
	return TUYA_BLE_SUCCESS;
}


/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_nv_init(void)
{
	return TUYA_BLE_SUCCESS;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_nv_erase(uint32_t addr,uint32_t size)
{
	return TUYA_BLE_SUCCESS;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_nv_write(uint32_t addr,const uint8_t *p_src, uint32_t size)
{
	return TUYA_BLE_SUCCESS;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_nv_read(uint32_t addr,uint8_t *p_dest, uint32_t size)
{
	return TUYA_BLE_SUCCESS;
}

/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK void tuya_ble_nv_erase_async(uint32_t addr,uint32_t size,void *p_context,tuya_ble_nv_async_callback_t callback)
{
	
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK void tuya_ble_nv_write_async(uint32_t addr,const uint8_t * p_src, uint32_t size,void *p_context,tuya_ble_nv_async_callback_t callback)
{
	
}

/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK void tuya_ble_nv_read_async(uint32_t addr,uint8_t * p_dest, uint32_t size,void *p_context,tuya_ble_nv_async_callback_t callback)
{

}

/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_common_uart_init(void)
{
    return TUYA_BLE_SUCCESS;
}

/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_common_uart_send_data(const uint8_t *p_data,uint16_t len)
{
    return TUYA_BLE_SUCCESS;
}


/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK bool tuya_ble_os_task_create(void **pp_handle, const char *p_name, void (*p_routine)(void *),void *p_param, uint16_t stack_size, uint16_t priority)
{
	return true;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK bool tuya_ble_os_task_delete(void *p_handle)
{
	return true;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK bool tuya_ble_os_task_suspend(void *p_handle)
{
	return true;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK bool tuya_ble_os_task_resume(void *p_handle)
{
	return true;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK bool tuya_ble_os_msg_queue_create(void **pp_handle, uint32_t msg_num, uint32_t msg_size)
{
	return true;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK bool tuya_ble_os_msg_queue_delete(void *p_handle)
{
	return true;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK bool tuya_ble_os_msg_queue_peek(void *p_handle, uint32_t *p_msg_num)
{
	return true;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK bool tuya_ble_os_msg_queue_send(void *p_handle, void *p_msg, uint32_t wait_ms)
{
	return true;
}
/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK bool tuya_ble_os_msg_queue_recv(void *p_handle, void *p_msg, uint32_t wait_ms)
{
	return true;
}


/**
 *@brief    
 *@param    
 *        
 *@note     
 *           
 * */
__TUYA_BLE_WEAK bool tuya_ble_event_queue_send_port(tuya_ble_evt_param_t *evt, uint32_t wait_ms)
{
	return true;
}



/**
    * @brief  128 bit AES ECB encryption on speicified plaintext and keys
    * @param  input    specifed plain text to be encypted
    * @param  input_len    byte length of the data to be descrypted, must be multiples of 16
    * @param  key          keys to encrypt the plaintext
    * @param  output    output buffer to store encrypted data
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note   least significant octet of encrypted data corresponds to encypted[0]
    */
__TUYA_BLE_WEAK bool tuya_ble_aes128_ecb_encrypt(uint8_t *key,uint8_t *input,uint16_t input_len,uint8_t *output)
{
	return true;
}

/**
    * @brief  128 bit AES ECB decryption on speicified encrypted data and keys
    * @param  input    specifed encypted data to be decypted
    * @param  input_len    byte length of the data to be descrypted, must be multiples of 16
    * @param  key          keys to decrypt the data
    * @param  output    output buffer to store plain data
    * @return decryption results
    * @retval true      successful
    * @retval false     fail
    * @note   least significant octet of encrypted data corresponds to encypted[0]
*/
__TUYA_BLE_WEAK bool tuya_ble_aes128_ecb_decrypt(uint8_t *key,uint8_t *input,uint16_t input_len,uint8_t *output)
{
	return true;
}
/**
    * @brief  128 bit AES CBC encryption on speicified plaintext and keys
    * @param  input    specifed plain text to be encypted
    * @param  key          keys to encrypt the plaintext
    * @param  output    output buffer to store encrypted data
    * @param  iv         initialization vector (IV) for CBC mode
    * @param  input_len    byte length of the data to be descrypted, must be multiples of 16
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note   least significant octet of encrypted data corresponds to encypted[0]
    */
__TUYA_BLE_WEAK bool tuya_ble_aes128_cbc_encrypt(uint8_t *key,uint8_t *iv,uint8_t *input,uint16_t input_len,uint8_t *output)
{
	return true;
}
/**
    * @brief  128 bit AES CBC descryption on speicified plaintext and keys
    * @param  input    specifed encypted data to be decypted
    * @param  key          keys to decrypt the data
    * @param  output    output buffer to store plain data
    * @param  iv         initialization vector (IV) for CBC mode
    * @param  input_len    byte length of the data to be descrypted, must be multiples of 16
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note   least significant octet of encrypted data corresponds to encypted[0]
    */
__TUYA_BLE_WEAK bool tuya_ble_aes128_cbc_decrypt(uint8_t *key,uint8_t *iv,uint8_t *input,uint16_t input_len,uint8_t *output)
{
	return true;
}
/**
    * @brief  MD5 checksum 
    * @param  input    specifed plain text to be encypted
    * @param  output    output buffer to store md5 result data,output data len is always 16
    * @param  input_len    byte length of the data to be encypted
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note   
    */
__TUYA_BLE_WEAK bool tuya_ble_md5_crypt(uint8_t *input,uint16_t input_len,uint8_t *output)
{
	return true;
}

/**
    * @brief          This function calculates the full generic HMAC
    *                 on the input buffer with the provided key.
    * @param  key      The HMAC secret key.
    * @param  key_len  The length of the HMAC secret key in Bytes.
    * @param  input    specifed plain text to be encypted
    * @param  output    output buffer to store the result data
    * @param  input_len    byte length of the data to be encypted
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note   
    */
__TUYA_BLE_WEAK bool tuya_ble_hmac_sha1_crypt(const uint8_t *key, uint32_t key_len, const uint8_t *input, uint32_t input_len, uint8_t *output)
{
	return true;
}

/**
    * @brief          This function calculates the full generic HMAC
    *                 on the input buffer with the provided key.
    * @param  key      The HMAC secret key.
    * @param  key_len  The length of the HMAC secret key in Bytes.
    * @param  input    specifed plain text to be encypted
    * @param  output    output buffer to store the result data
    * @param  input_len    byte length of the data to be encypted
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note   
    */
__TUYA_BLE_WEAK bool tuya_ble_hmac_sha256_crypt(const uint8_t *key, uint32_t key_len, const uint8_t *input, uint32_t input_len, uint8_t *output)
{
	return true;
}


/**
 * \brief    Allocate a memory block with required size.
 *
 *
 * \param[in]   size     Required memory size.
 *
 * \return     The address of the allocated memory block. If the address is NULL, the
 *             memory allocation failed.
 */
__TUYA_BLE_WEAK void *tuya_ble_port_malloc( uint32_t size );



/**
 *
 * \brief    Free a memory block that had been allocated.
 *
 * \param[in]   pv     The address of memory block being freed.
 *
 * \return     None.
 */
__TUYA_BLE_WEAK void tuya_ble_port_free( void *pv );




