/* ----------------------------------------------------------------------------
 * Copyright (c) 2018 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * app_config.c
 * - Application configuration source file
 * ------------------------------------------------------------------------- */

#include "app.h"

static const struct ke_msg_handler appm_default_state[] =
{
    { KE_MSG_DEFAULT_HANDLER, (ke_msg_func_t)MsgHandler_Notify }
};

/* Use the state and event handler definition for all states. */
static const struct ke_state_handler appm_default_handler
    = KE_STATE_HANDLER(appm_default_state);

/* Defines a place holder for all task instance's state */
ke_state_t appm_state[APP_IDX_MAX];

static const struct ke_task_desc TASK_DESC_APP = {
    NULL,       &appm_default_handler,
    appm_state, 0,
    APP_IDX_MAX
};

struct gapm_set_dev_config_cmd devConfigCmd =
{
    .operation = GAPM_SET_DEV_CONFIG,
    .role = GAP_ROLE_PERIPHERAL,
    .renew_dur = GAPM_DEFAULT_RENEW_DUR,
    .addr.addr = APP_BD_ADDRESS,
    .irk.key = APP_IRK,
    .addr_type = APP_BD_ADDRESS_TYPE,
#ifdef SECURE_CONNECTION
    .pairing_mode = (GAPM_PAIRING_SEC_CON | GAPM_PAIRING_LEGACY),
#else
    .pairing_mode = GAPM_PAIRING_LEGACY,
#endif
    .gap_start_hdl = GAPM_DEFAULT_GAP_START_HDL,
    .gatt_start_hdl = GAPM_DEFAULT_GATT_START_HDL,
    .att_and_ext_cfg = GAPM_DEFAULT_ATT_CFG,
    .sugg_max_tx_octets = GAPM_DEFAULT_TX_OCT_MAX,
    .sugg_max_tx_time = GAPM_DEFAULT_TX_TIME_MAX,
    .max_mtu = GAPM_DEFAULT_MTU_MAX,
    .max_mps = GAPM_DEFAULT_MPS_MAX,
    .max_nb_lecb = GAPM_DEFAULT_MAX_NB_LECB,
    .audio_cfg = GAPM_DEFAULT_AUDIO_CFG,
    .tx_pref_rates = GAP_RATE_ANY,
    .rx_pref_rates = GAP_RATE_ANY
};

struct gapm_start_advertise_cmd advertiseCmd =
{
    .op = {
        .code = GAPM_ADV_UNDIRECT,
        .addr_src = GAPM_STATIC_ADDR,
        .state = 0
    },
    .intv_min = GAPM_DEFAULT_ADV_INTV_MIN,
    .intv_max = GAPM_DEFAULT_ADV_INTV_MAX,
    .channel_map = GAPM_DEFAULT_ADV_CHMAP,
    .info.host = {
        .mode = GAP_GEN_DISCOVERABLE,
        .adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY
        /* ADV_DATA and SCAN_RSP data are set in APP_BLE_Initialize() */
    }
};

struct gap_dev_name_buff
{
    /// name length
    uint16_t length;
    /// name value
    uint8_t value[APP_DEVICE_NAME_LEN];
};

const struct gap_dev_name_buff getDevInfoCfmName =
{
    .length = APP_DEVICE_NAME_LEN,
    .value  = APP_DEVICE_NAME
};

const union gapc_dev_info_val getDevInfoCfmAppearance =
{
    .appearance = APP_DEVICE_APPEARANCE
};

const union gapc_dev_info_val getDevInfoCfmSlvParams =
{
    .slv_params = {APP_PREF_SLV_MIN_CON_INTERVAL,
                   APP_PREF_SLV_MAX_CON_INTERVAL,
                   APP_PREF_SLV_LATENCY,
                   APP_PREF_SLV_SUP_TIMEOUT}
};

const union gapc_dev_info_val* getDevInfoCfm[] =
{
    [GAPC_DEV_NAME] = (const union gapc_dev_info_val*) &getDevInfoCfmName,
    [GAPC_DEV_APPEARANCE] = &getDevInfoCfmAppearance,
    [GAPC_DEV_SLV_PREF_PARAMS] = &getDevInfoCfmSlvParams
};

/* ----------------------------------------------------------------------------
 * Function      : void Device_Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Configure hardware and initialize BLE stack
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Device_Initialize(void)
{
    /* Mask all interrupts */
    __set_PRIMASK(PRIMASK_DISABLE_INTERRUPTS);

    /* Disable all interrupts and clear any pending interrupts */
    Sys_NVIC_DisableAllInt();
    Sys_NVIC_ClearAllPendingInt();

    /* Test DIO12 to pause the program to make it easy to re-flash */
    DIO->CFG[RECOVERY_DIO] = DIO_MODE_INPUT  | DIO_WEAK_PULL_UP |
                             DIO_LPF_DISABLE | DIO_6X_DRIVE;

    if(DIO_DATA->ALIAS[RECOVERY_DIO] == 0)
    {
        /* Erase the bond list from NVR2 flash */
        BondList_RemoveAll();
    }

    while (DIO_DATA->ALIAS[RECOVERY_DIO] == 0);

    /* Configure the current trim settings for VCC, VDDA */
    ACS_VCC_CTRL->ICH_TRIM_BYTE  = VCC_ICHTRIM_16MA_BYTE;
    ACS_VDDA_CP_CTRL->PTRIM_BYTE = VDDA_PTRIM_16MA_BYTE;

    /* Start 48 MHz XTAL oscillator */
    ACS_VDDRF_CTRL->ENABLE_ALIAS = VDDRF_ENABLE_BITBAND;
    ACS_VDDRF_CTRL->CLAMP_ALIAS  = VDDRF_DISABLE_HIZ_BITBAND;

    /* Wait until VDDRF supply has powered up */
    while (ACS_VDDRF_CTRL->READY_ALIAS != VDDRF_READY_BITBAND);

    /* Disable RF TX power amplifier supply voltage and
    * connect the switched output to VDDRF regulator */
    ACS_VDDPA_CTRL->ENABLE_ALIAS = VDDPA_DISABLE_BITBAND;
    ACS_VDDPA_CTRL->VDDPA_SW_CTRL_ALIAS    = VDDPA_SW_VDDRF_BITBAND;

    /* Enable RF power switches */
    SYSCTRL_RF_POWER_CFG->RF_POWER_ALIAS   = RF_POWER_ENABLE_BITBAND;

    /* Remove RF isolation */
    SYSCTRL_RF_ACCESS_CFG->RF_ACCESS_ALIAS = RF_ACCESS_ENABLE_BITBAND;

    /* Set radio clock accuracy in ppm */
    BLE_DeviceParam_Set_ClockAccuracy(RADIO_CLOCK_ACCURACY);

    /* Start the 48 MHz oscillator without changing the other register bits */
    RF->XTAL_CTRL = ((RF->XTAL_CTRL & ~XTAL_CTRL_DISABLE_OSCILLATOR) |
                     XTAL_CTRL_REG_VALUE_SEL_INTERNAL);

    /* Enable the 48 MHz oscillator divider using the desired prescale value */
    RF_REG2F->CK_DIV_1_6_CK_DIV_1_6_BYTE = CK_DIV_1_6_PRESCALE_6_BYTE;

    /* Wait until 48 MHz oscillator is started */
    while (RF_REG39->ANALOG_INFO_CLK_DIG_READY_ALIAS !=
           ANALOG_INFO_CLK_DIG_READY_BITBAND);

    /* Switch to (divided 48 MHz) oscillator clock */
    Sys_Clocks_SystemClkConfig(JTCK_PRESCALE_1   |
                               EXTCLK_PRESCALE_1 |
                               SYSCLK_CLKSRC_RFCLK);

    /* Configure clock dividers */
    CLK->DIV_CFG0 = (SLOWCLK_PRESCALE_8 | BBCLK_PRESCALE_1 |
                     USRCLK_PRESCALE_1);
    CLK->DIV_CFG2 = (CPCLK_PRESCALE_8 | DCCLK_PRESCALE_2);

    BBIF->CTRL    = (BB_CLK_ENABLE | BBCLK_DIVIDER_8 | BB_WAKEUP);

    /* Configure DIOs */
    Sys_DIO_Config(LED_DIO_NUM, DIO_MODE_GPIO_OUT_0);

    /* Seed the random number generator */
    srand(1);

    /* Initialize the kernel and Bluetooth stack */
    Kernel_Init(0);
    BLE_InitNoTL(0);
	
	/* Set radio output power of RF */
    Sys_RFFE_SetTXPower(OUTPUT_POWER_DBM);

    /* Create the application task handler */
    ke_task_create(TASK_APP, &TASK_DESC_APP);

    /* Enable Bluetooth related interrupts */
    NVIC->ISER[1] = (NVIC_BLE_CSCNT_INT_ENABLE      |
                     NVIC_BLE_SLP_INT_ENABLE        |
                     NVIC_BLE_RX_INT_ENABLE         |
                     NVIC_BLE_EVENT_INT_ENABLE      |
                     NVIC_BLE_CRYPT_INT_ENABLE      |
                     NVIC_BLE_ERROR_INT_ENABLE      |
                     NVIC_BLE_GROSSTGTIM_INT_ENABLE |
                     NVIC_BLE_FINETGTIM_INT_ENABLE  |
                     NVIC_BLE_SW_INT_ENABLE);

    /* Stop masking interrupts */
    __set_PRIMASK(PRIMASK_ENABLE_INTERRUPTS);
    __set_FAULTMASK(FAULTMASK_ENABLE_INTERRUPTS);
}

void APP_SetAdvScanData(void)
{
    uint8_t companyID[] = APP_COMPANY_ID;
    uint8_t devName[]   = APP_DEVICE_NAME;

    /* Set advertising data as device name + company ID */
    advertiseCmd.info.host.adv_data_len = 0;
    GAPM_AddAdvData(GAP_AD_TYPE_COMPLETE_NAME, devName,
                    APP_DEVICE_NAME_LEN, advertiseCmd.info.host.adv_data,
                    &advertiseCmd.info.host.adv_data_len);
    GAPM_AddAdvData(GAP_AD_TYPE_MANU_SPECIFIC_DATA, companyID,
                    APP_COMPANY_ID_LEN, advertiseCmd.info.host.adv_data,
                    &advertiseCmd.info.host.adv_data_len);

    /* Set scan response data as company ID */
    advertiseCmd.info.host.scan_rsp_data_len = 0;
    GAPM_AddAdvData(GAP_AD_TYPE_MANU_SPECIFIC_DATA, companyID,
                    APP_COMPANY_ID_LEN, advertiseCmd.info.host.scan_rsp_data,
                    &advertiseCmd.info.host.scan_rsp_data_len);
}

void APP_SetConnectionCfmParams(uint8_t conidx, struct gapc_connection_cfm* cfm)
{
    cfm->svc_changed_ind_enable = 0;
    cfm->ltk_present = false;
#ifdef SECURE_CONNECTION
        cfm->pairing_lvl = GAP_PAIRING_BOND_SECURE_CON;
#else
    cfm->pairing_lvl = GAP_PAIRING_BOND_UNAUTH;
#endif

#if CFG_BOND_LIST_IN_NVR2
    if(GAPC_IsBonded(conidx))
    {
        cfm->ltk_present = true;
        Device_Param_Read(PARAM_ID_CSRK, cfm->lcsrk.key);
        memcpy(cfm->rcsrk.key, GAPC_GetBondInfo(conidx)->CSRK, KEY_LEN);
        cfm->lsign_counter = 0xFFFFFFFF;
        cfm->rsign_counter = 0;
    }
#endif

    PRINTF("  connectionCfm->ltk_present = %d \n\r", cfm->ltk_present);
}

/* ----------------------------------------------------------------------------
 * Function      : Device_Param_Prepare(struct app_device_param * param)
 * ----------------------------------------------------------------------------
 * Description   : This function allows the application to overwrite a few BLE
 *                 parameters (BD address and keys) without having to write
 *                 data into RSL10 flash (NVR3). This function is called by the
 *                 stack and it's useful for debugging and testing purposes.
 * Inputs        : - param    - pointer to the parameters to be configured
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Device_Param_Prepare(app_device_param_t *param)
{
    param->device_param_src_type = APP_BLE_DEV_PARAM_SOURCE;

    if(param->device_param_src_type == APP_PROVIDED)
    {
        uint8_t temp_bleAddress[6] = APP_BD_ADDRESS;
        uint8_t temp_irk[16] = APP_IRK;
        uint8_t temp_csrk[16] = APP_CSRK;
        uint8_t temp_privateKey[32] = APP_PRIVATE_KEY;
        uint8_t temp_publicKey_x[32] = APP_PUBLIC_KEY_X;
        uint8_t temp_publicKey_y[32] = APP_PUBLIC_KEY_Y;

        memcpy(param->bleAddress, temp_bleAddress, 6);
        memcpy(param->irk, temp_irk, 16);
        memcpy(param->csrk, temp_csrk, 16);
        memcpy(param->privateKey, temp_privateKey, 32);
        memcpy(param->publicKey_x, temp_publicKey_x, 32);
        memcpy(param->publicKey_y, temp_publicKey_y, 32);
    }
}

