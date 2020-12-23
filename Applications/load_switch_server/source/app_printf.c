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

/* ----------------------------------------------------------------------------
 * PRINTF call redirection
 * ------------------------------------------------------------------------- */

#if (CFG_APP_PRINTF_INTERFACE == APP_PRINTF_INTERFACE_RTT)

static uint8_t app_printf_rtt_up_buffer[CFG_APP_PRINTF_RTT_BUFFER_SIZE] __ALIGNED(8);

void APP_PRINTF_InitializeRTT(void)
{
	SEGGER_RTT_Init();

	/* Set Terminal to use application allocated buffer for extra flexibility
	 * when linking with pre-build RTT library.
	 */
	SEGGER_RTT_ConfigUpBuffer(0, "Terminal", app_printf_rtt_up_buffer,
			CFG_APP_PRINTF_RTT_BUFFER_SIZE, SEGGER_RTT_MODE_DEFAULT);
}

#endif /* if (CFG_APP_PRINTF_INTERFACE == APP_PRINTF_INTERFACE_RTT) */

#if (CFG_APP_PRINTF_INTERFACE == APP_PRINTF_INTERFACE_UART)

#include <stdio.h>
#include <stdarg.h>

#define UART_BAUD_RATE             CFG_APP_PRINTF_UART_BAUDRATE
#define TX_BUFFER_SIZE             200
#define DMA_UART_TX                7
#define UART_TX                    APP_PIN_GIO_SPARE
#define UART_RX                    7
#define UART_TX_CFG                (DMA_DISABLE                | \
                                    DMA_ADDR_LIN               | \
                                    DMA_TRANSFER_M_TO_P        | \
                                    DMA_PRIORITY_0             | \
                                    DMA_DISABLE_INT_DISABLE    | \
                                    DMA_ERROR_INT_DISABLE      | \
                                    DMA_COMPLETE_INT_ENABLE    | \
                                    DMA_COUNTER_INT_DISABLE    | \
                                    DMA_START_INT_DISABLE      | \
                                    DMA_LITTLE_ENDIAN          | \
                                    DMA_SRC_ADDR_INC           | \
                                    DMA_SRC_WORD_SIZE_32       | \
                                    DMA_SRC_ADDR_STEP_SIZE_1   | \
                                    DMA_DEST_ADDR_STATIC       | \
                                    DMA_DEST_UART              | \
                                    DMA_DEST_WORD_SIZE_8)

void DMA_IRQ_FUNC(DMA_UART_TX)(void);

static uint8_t UARTTXBuffer[TX_BUFFER_SIZE];
static uint8_t tx_busy = 0;

void APP_PRINTF_UartInit(void)
{
    Sys_UART_DIOConfig(DIO_6X_DRIVE | DIO_WEAK_PULL_UP | DIO_LPF_ENABLE,
                       UART_TX, UART_RX);
    Sys_UART_Enable(SystemCoreClock, UART_BAUD_RATE, UART_DMA_MODE_ENABLE);

    Sys_DMA_ChannelConfig(DMA_UART_TX, UART_TX_CFG, 1, 0,
                          (uint32_t)UARTTXBuffer, (uint32_t)&UART->TX_DATA);
    NVIC_SetPriority(DMA_IRQn(DMA_UART_TX), 0x4);
    NVIC_EnableIRQ(DMA_IRQn(DMA_UART_TX));

    tx_busy = 0;
}

void APP_PRINTF_UartSleep(void)
{
    /* Wait for UART to transmit all data. */
    while (tx_busy);

    /* Wait for UART buffer to empty last character.
     * tx_busy flag is cleared when last byte is written to TX buffer but it
     * still needs to be transmitted.
     */
    Sys_Delay_ProgramROM(2000);

    Sys_UART_Disable();

    /* Set UART DIO pins to disabled state with no internal pull resistors. */
    Sys_DIO_Config(UART_TX, DIO_MODE_DISABLE);
    Sys_DIO_Config(UART_RX, DIO_MODE_DISABLE);
}

void DMA_IRQ_FUNC(DMA_UART_TX)(void)
{
    tx_busy = 0;
    Sys_DMA_ClearChannelStatus(DMA_UART_TX);
}

/**
 * Printf function for the UART debug port.
 *
 * @pre
 * The UART and the DMA has been initialized.
 *
 * @param narg
 * Number of arguments to print.
 *
 * @param sFormat
 * Pointer to the argument list.
 *
 * @return
 * Number of bytes to be printed by the debug port.
 */
int UART_printf(const int narg, const char *sFormat, ...)
{
    va_list arg;
    int done = 0;

    /* Check until the DMA is not busy */
    while (tx_busy == 1)
    {
        Sys_Watchdog_Refresh();
    }

    if (narg > 1)
    {
        /* Use sprint if there is more than one argument */
        va_start(arg, sFormat);
        done = vsprintf((char*)UARTTXBuffer, sFormat, arg);
        va_end(arg);
        DMA_CTRL1[DMA_UART_TX].TRANSFER_LENGTH_SHORT = strlen((const char *)UARTTXBuffer);
    }
    else
    {
        /* Use simple memcpy if there is one argument */
        DMA_CTRL1[DMA_UART_TX].TRANSFER_LENGTH_SHORT = strlen(sFormat);
        memcpy(UARTTXBuffer, sFormat, DMA_CTRL1[DMA_UART_TX].TRANSFER_LENGTH_SHORT);
    }

    /* The DMA is busy */
    tx_busy = 1;
    Sys_DMA_ChannelEnable(DMA_UART_TX);

    return done;
}

#endif /* if (CFG_APP_PRINTF_INTERFACE == APP_PRINTF_INTERFACE_UART) */

