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
 * @file smartshot_assert.c
 *
 */
#include <app_assert.h>
#include <app_printf.h>
#include <rsl10.h>

/* ----------------------------------------------------------------------------
 * Define & Type declaration
 * ------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------
 * Public Global variables Declaration
 * ------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------
 * Private Global variables Declaration
 * ------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------
 * Private Function Forward Declaration
 * ------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------
 * Public Function Definition
 * ------------------------------------------------------------------------- */

void AssertFailed(const char *p_file, int line)
{
    PRINTF("\r\n%s:%d Assertion Failed!\r\n", p_file, line);

    while (1)
    {
        Sys_Watchdog_Refresh();
    }
}

/* ----------------------------------------------------------------------------
 * End of File
 * ------------------------------------------------------------------------- */
