/* ----------------------------------------------------------------------------
 * Copyright (c) 2018 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * This module is derived in part from example code provided by RivieraWaves
 * and as such the underlying code is the property of RivieraWaves [a member
 * of the CEVA, Inc. group of companies], together with additional code which
 * is the property of ON Semiconductor. The code (in whole or any part) may not
 * be redistributed in any form without prior written permission from
 * ON Semiconductor.
 *
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * app.h
 * - Main application header
 * ------------------------------------------------------------------------- */

#ifndef APP_H
#define APP_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif    /* ifdef __cplusplus */

/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/
#include <rsl10.h>
#include <rsl10_protocol.h>
#include <ble_gap.h>
#include <ble_gatt.h>
#include <msg_handler.h>
#include <ble_bass.h>
#include "app_customss.h"
#include "app_bass.h"
#include <printf.h>

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/* APP Task messages */
enum appm_msg
{
    APPM_DUMMY_MSG = TASK_FIRST_MSG(TASK_ID_APP),
    APP_LED_TIMEOUT,
    APP_BATT_LEVEL_LOW
};

#define APP_IDX_MAX                     BLE_CONNECTION_MAX /* Number of APP Task Instances */

#define APP_BLE_DEV_PARAM_SOURCE        FLASH_PROVIDED_or_DFLT /* or APP_PROVIDED  */

/* If APP_BD_ADDRESS_TYPE == GAPM_CFG_ADDR_PUBLIC and APP_DEVICE_PARAM_SRC == FLASH_PROVIDED_or_DFLT
 * the bluetooth address is loaded from FLASH NVR3. Otherwise, this address is used. */
#define APP_BD_ADDRESS_TYPE              GAPM_CFG_ADDR_PRIVATE /* or GAPM_CFG_ADDR_PUBLIC*/
#ifndef APP_BD_ADDRESS
#define APP_BD_ADDRESS                  { 0x94, 0x11, 0x22, 0xff, 0xbb, 0xD5 }
#endif
#define APP_NB_PEERS                     4 /* 1-4 */

/* The number of standard profiles and custom services added in this application */
#define APP_NUM_STD_PRF                 1
#define APP_NUM_CUSTOM_SVC              1

#define LED_DIO_NUM                     6  /* DIO number that is connected to LED of EVB */
#define OUTPUT_POWER_DBM                0  /* RF output power in dBm */
#define RADIO_CLOCK_ACCURACY            20 /* RF Oscillator accuracy in ppm */

/* DIO number that is used for easy re-flashing (recovery mode) */
#define RECOVERY_DIO                    12

/* Timer setting in units of 10ms (kernel timer resolution) */
#define TIMER_SETTING_MS(MS)            (MS / 10)
#define TIMER_SETTING_S(S)              (S * 100)

/* Advertising data is composed by device name and company id */
#define APP_DEVICE_NAME                 "ble_periph_server_bond"
#define APP_DEVICE_NAME_LEN             (sizeof(APP_DEVICE_NAME) - 1)

/* Manufacturer info (ON SEMICONDUCTOR Company ID) */
#define APP_COMPANY_ID                    {0x62, 0x3}
#define APP_COMPANY_ID_LEN                2

#define APP_DEVICE_APPEARANCE           0
#define APP_PREF_SLV_MIN_CON_INTERVAL   8
#define APP_PREF_SLV_MAX_CON_INTERVAL   10
#define APP_PREF_SLV_LATENCY            0
#define APP_PREF_SLV_SUP_TIMEOUT        200

/* Application-provided IRK */
#define APP_IRK                         { 0x01, 0x23, 0x45, 0x68, 0x78, 0x9a, \
                                          0xbc, 0xde, 0x01, 0x23, 0x45, 0x68, \
                                          0x78, 0x9a, 0xbc, 0xde }

/* Application-provided CSRK */
#define APP_CSRK                        { 0x01, 0x23, 0x45, 0x68, 0x78, 0x9a, \
                                          0xbc, 0xde, 0x01, 0x23, 0x45, 0x68, \
                                          0x78, 0x9a, 0xbc, 0xde }

/* Application-provided private key */
#define APP_PRIVATE_KEY                 { 0xEC, 0x89, 0x3C, 0x11, 0xBB, 0x2E, \
                                          0xEB, 0x5C, 0x80, 0x88, 0x63, 0x57, \
                                          0xCC, 0xE2, 0x05, 0x17, 0x20, 0x75, \
                                          0x5A, 0x26, 0x3E, 0x8D, 0xCF, 0x26, \
                                          0x63, 0x1D, 0x26, 0x0B, 0xCE, 0x4D, \
                                          0x9E, 0x07 }

/* Application-provided public key X */
#define APP_PUBLIC_KEY_X                { 0x56, 0x09, 0x79, 0x1D, 0x5A, 0x5F, \
                                          0x4A, 0x5C, 0xFE, 0x89, 0x56, 0xEC, \
                                          0xE6, 0xF7, 0x92, 0x21, 0xAC, 0x93, \
                                          0x99, 0x10, 0x51, 0x82, 0xF4, 0xDD, \
                                          0x84, 0x07, 0x50, 0x99, 0xE7, 0xC2, \
                                          0xF1, 0xC8 }

/* Application-provided public key Y */
#define APP_PUBLIC_KEY_Y                { 0x40, 0x84, 0xB4, 0xA6, 0x08, 0x67, \
                                          0xFD, 0xAC, 0x81, 0x5D, 0xB0, 0x41, \
                                          0x27, 0x75, 0x9B, 0xA7, 0x92, 0x57, \
                                          0x0C, 0x44, 0xB1, 0x57, 0x7C, 0x76, \
                                          0x5B, 0x56, 0xF0, 0xBA, 0x03, 0xF4, \
                                          0xAA, 0x67 }

/* ---------------------------------------------------------------------------
* Function prototype definitions
* --------------------------------------------------------------------------*/
void Device_Initialize(void);

void APP_SetAdvScanData(void);

void APP_SetConnectionCfmParams(uint8_t conidx, struct gapc_connection_cfm* cfm);

void APP_LED_Timeout_Handler(ke_msg_id_t const msg_id, void const *param,
                       ke_task_id_t const dest_id, ke_task_id_t const src_id);

void APP_GAPM_GATTM_Handler(ke_msg_id_t const msg_id, void const *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id);

void APP_GAPC_Handler(ke_msg_id_t const msg_id, void const *param,
                      ke_task_id_t const dest_id, ke_task_id_t const src_id);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* APP_H */
