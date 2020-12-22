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
 * app_bass.c
 * - BASS Application-specific source file
 * ------------------------------------------------------------------------- */
#include <app.h>
#include <app_bass.h>

/* ----------------------------------------------------------------------------
 * Function      : void APP_BASS_SetBatMonAlarm(uint32_t supplyThresholdCfg)
 * ----------------------------------------------------------------------------
 * Description   : Configure ADC to read VBAT/2 and trigger a BATMON alarm
 *                 interrupt when adc level < supplyThresholdCfg. The interrupt
 *                 is handled by ADC_BATMON_IRQHandler.
 * Inputs        : supplyThresholdCfg  - BATMON supply threshold configuration
 * Outputs       : None
 * Assumptions   : Alarm count value is set to 100. ADC channels used are:
 *                 ADC_BATMON_CH and ADC_GND_CH.
 * ------------------------------------------------------------------------- */
void APP_BASS_SetBatMonAlarm(uint32_t supplyThresholdCfg)
{
    /* Set the ADC configuration */
    Sys_ADC_Set_Config(ADC_VBAT_DIV2_NORMAL | ADC_NORMAL | ADC_PRESCALE_1280H);

    /* Enable BATMON alarm interrupt */
    Sys_ADC_Set_BATMONIntConfig(INT_DIS_ADC | INT_EBL_BATMON_ALARM);

    /* Set the BATMON alarm count, threshold and channel configuration */
    Sys_ADC_Set_BATMONConfig((100 << ADC_BATMON_CFG_ALARM_COUNT_VALUE_Pos) |
                             (supplyThresholdCfg <<
                              ADC_BATMON_CFG_SUPPLY_THRESHOLD_Pos) |
                              BATMON_CH(ADC_BATMON_CH));

    /* Configure ADC channel "BATT_LEVEL_ADC_CH" to measure VBAT/2 */
    Sys_ADC_InputSelectConfig(ADC_BATMON_CH, (ADC_POS_INPUT_VBAT_DIV2 |
                              ADC_NEG_INPUT_GND));

    /* Configure both input selection for an ADC channel to GND so the OFFSET is
     * subtracted automatically to result. */
    Sys_ADC_InputSelectConfig(ADC_GND_CH, ADC_POS_INPUT_GND |
                              ADC_NEG_INPUT_GND);

    NVIC_EnableIRQ(ADC_BATMON_IRQn);
}

/* ----------------------------------------------------------------------------
 * Function      : void APP_BASS_ReadBatteryLevel(uint8_t bas_nb)
 * ----------------------------------------------------------------------------
 * Description   : Calculate the current battery level in a scale of [0,100],
 *                 where 0% = 1.1V and 100% = 1.4V. The ADC measurements are
 *                 averaged 16 times with a 5ms delay in between.
 * Inputs        : uint8_t bas_nb   - Battery instance [0,1].
 * Outputs       : An integer in the [0,100] range.
 * Assumptions   : Return the same battery value for any bas_nb argument.
 * ------------------------------------------------------------------------- */
uint8_t APP_BASS_ReadBatteryLevel(uint8_t bas_nb)
{
    uint32_t adc_avg = 0;
    uint8_t battLevelPercent;

    for(uint8_t i = 0; i < 16; i++) /* Perform 16 ADC reads */
    {
        adc_avg += ADC->DATA_TRIM_CH[ADC_BATMON_CH];
        Sys_Delay_ProgramROM((uint32_t)(0.005 * SystemCoreClock)); /* 5ms delay */
    }

    adc_avg = adc_avg >> 4; /* Average 16 reads */

    /* Calculate percentage battery level */
    battLevelPercent = (uint8_t)(((adc_avg - VBAT_1p1V_MEASURED) * 100) /
                                 (VBAT_1p4V_MEASURED - VBAT_1p1V_MEASURED));

    return ((battLevelPercent <= 100) ? battLevelPercent : 100);
}

/* ----------------------------------------------------------------------------
 * Function      : void APP_BASS_BattLevelReached_Handler(
 *                                     ke_msg_id_t const msg_id,
                                       void const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle the APP_BATT_LEVEL_LOW event triggered by the
 *                 BATMON alarm interrupt. The application is trapped in this
 *                 function until the battery level rises above the threshold.
 *                 Periodic services' notifications and the LED activity are
 *                 temporarily disabled. Other BLE activity continues normally.
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void APP_BASS_BattLevelLow_Handler(ke_msg_id_t const msg_id,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    /* Disable periodic notifications to save energy */
    BASS_NotifyOnTimeout(0);
    CUSTOMSS_NotifyOnTimeout(0);

    /* Turn off LED activity */
    ke_timer_clear(APP_LED_TIMEOUT, TASK_APP);
    Sys_GPIO_Set_Low(LED_DIO_NUM);

    /* Trap application here until battery is recharged.
     * BLE communication continues normally. */
    while(APP_BASS_ReadBatteryLevel(0) <= BATT_LEVEL_LOW_THRESHOLD_PERCENT)
    {
        Sys_Watchdog_Refresh();
        Kernel_Schedule();
    }

    /* Restart LED timer */
    ke_timer_set(APP_LED_TIMEOUT, TASK_APP, TIMER_SETTING_MS(200));

    /* Re-enable periodic notifications */
    BASS_NotifyOnTimeout(TIMER_SETTING_S(5));
    CUSTOMSS_NotifyOnTimeout(TIMER_SETTING_S(5));

    /* Re-enable BATMON interrupt */
    Sys_ADC_Clear_BATMONStatus();
    ADC->BATMON_COUNT_VAL; /* Read this register in order to clear the counter */
    NVIC_EnableIRQ(ADC_BATMON_IRQn);
}

/* ----------------------------------------------------------------------------
 * Function      : void ADC_BATMON_IRQHandler(void)
 * ----------------------------------------------------------------------------
 * Description   : Handle BATMON alarm interrupt when the VBAT falls under the
 *                 configured threshold. Trigger an APP_BATT_LEVEL_LOW event.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void ADC_BATMON_IRQHandler(void)
{
    /* Disable BATMON interrupt temporarily */
    NVIC_DisableIRQ(ADC_BATMON_IRQn);

    /* Trigger APP_BATT_LEVEL_LOW event  */
    ke_msg_send_basic(APP_BATT_LEVEL_LOW, TASK_APP, TASK_APP);
}
