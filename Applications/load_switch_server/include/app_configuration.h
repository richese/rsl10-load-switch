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
 * @file app_configuration.h
 *
 */

#ifndef APP_CONFIGURATION_H
#define APP_CONFIGURATION_H

/* ----------------------------------------------------------------------------
 * Configuration Defines
 * --------------------------------------------------------------------------*/

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Application Settings

// <s> Application Name
#define CFG_APP_NAME  "load_switch_server"

// </h>

// <h> FOTA Application Information

// <s.6> Application Identifier
// <i> Short identification string for this application.
// <i> Must be 6 characters long.
#define CFG_FOTA_VER_ID  "LS_SRV"

// <h> Version

// <o> Major <0-15>
// <i> Range: 0 - 15
#define CFG_FOTA_VER_MAJOR  0

// <o> Minor <0-15>
// <i> Range: 0 - 15
#define CFG_FOTA_VER_MINOR  0

// <o> Revision <0-255>
// <i> Range: 0 - 255
#define CFG_FOTA_VER_REVISION  1

// </h>
// </h>

// <h> Debugging Options

// <q> Enable ASSERT
#define CFG_APP_ASSERT_ENABLED  (1)

// <o> PRINTF Output Interface
//    <0=> Disabled
//    <1=> UART
//    <2=> SEGGER RTT
//    <3=> SEGGER System View (Host)
//    <4=> SEGGER System View (Target)
#define CFG_APP_PRINTF_INTERFACE  (2)

// <o> PRINTF UART Baudrate
// <i> Baudrate to use for UART when UART Output interface is used.
#define CFG_APP_PRINTF_UART_BAUDRATE  230400

// <o> RTT Terminal Up Buffer Size [bytes]
// <i> Size of Target -> Host message buffer
#define CFG_APP_PRINTF_RTT_BUFFER_SIZE  (1024)

// </h>

// <<< end of configuration section >>>

/* ----------------------------------------------------------------------------
 * Compile time preprocessor checks
 * --------------------------------------------------------------------------*/

#endif /* APP_CONFIGURATION_H */

/* ----------------------------------------------------------------------------
 * End of File
 * --------------------------------------------------------------------------*/
