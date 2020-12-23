/* ----------------------------------------------------------------------------
 * Copyright (c) 2020 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ------------------------------------------------------------------------- */

/**
 * @file app_printf.h
 *
 */

#ifndef APP_PRINTF_H
#define APP_PRINTF_H

#include <app_configuration.h>
#include <stdbool.h>
#include <stdint.h>


/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

#define APP_PRINTF_INTERFACE_DISABLED         0
#define APP_PRINTF_INTERFACE_UART             1
#define APP_PRINTF_INTERFACE_RTT              2
#define APP_PRINTF_INTERFACE_SYSVIEW_HOST     3
#define APP_PRINTF_INTERFACE_SYSVIEW_TARGET   4

/* By default set in app_confg.h */
#ifndef CFG_APP_PRINTF_INTERFACE
#define CFG_APP_PRINTF_INTERFACE  APP_PRINTF_INTERFACE_DISABLED
#endif /* CFG_PRINTF_INTERFACE */

#if CFG_APP_PRINTF_INTERFACE == APP_PRINTF_INTERFACE_DISABLED

#define PRINTF(...)
#define APP_PRINTF_INIT()
#define APP_PRINTF_SLEEP()

#elif CFG_APP_PRINTF_INTERFACE == APP_PRINTF_INTERFACE_UART

#define THREE_BLOCK_APPN(x, y, z)       x##y##z
#define DMA_IRQn(x)                     THREE_BLOCK_APPN(DMA, x, _IRQn)
#define DMA_IRQ_FUNC(x)                 THREE_BLOCK_APPN(DMA, x, _IRQHandler)
#define VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

int UART_printf(const int narg, const char *sFormat, ...);
void DMA_IRQ_FUNC(DMA_UART_TX)(void);
void APP_PRINTF_UartInit(void);
void APP_PRINTF_UartSleep(void);

#define PRINTF(...) UART_printf(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
#define APP_PRINTF_INIT() APP_PRINTF_UartInit()
#define APP_PRINTF_SLEEP() APP_PRINTF_UartSleep()

#elif CFG_APP_PRINTF_INTERFACE == APP_PRINTF_INTERFACE_RTT

#include <SEGGER_RTT.h>

#define APP_PRINTF_INIT() APP_PRINTF_InitializeRTT()
#define APP_PRINTF_SLEEP()
#define PRINTF(...) SEGGER_RTT_printf(0, __VA_ARGS__)

void APP_PRINTF_InitializeRTT(void);

#elif CFG_APP_PRINTF_INTERFACE == APP_PRINTF_INTERFACE_SYSVIEW_HOST

#include <SEGGER_SYSVIEW.h>
#define APP_PRINTF_INIT()
#define APP_PRINTF_SLEEP()
/* Formatting of the string is off loaded to Host PC application. */
#define PRINTF(...) SEGGER_SYSVIEW_PrintfHost(__VA_ARGS__)

#elif CFG_APP_PRINTF_INTERFACE == APP_PRINTF_INTERFACE_SYSVIEW_TARGET

#include <SEGGER_SYSVIEW.h>
#define APP_PRINTF_INIT()
#define APP_PRINTF_SLEEP()
/* Formatting of the string is done in the target device (RSL10). */
#define PRINTF(...) SEGGER_SYSVIEW_PrintfTarget(__VA_ARGS__)

#endif


/* ---------------------------------------------------------------------------
* Function prototype definitions
* --------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* APP_PRINTF_H */
