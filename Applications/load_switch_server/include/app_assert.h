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
 * @file app_assert.h
 *
 * Contains custom assert implementation for use by application.
 */

#ifndef APP_ASSERT_H
#define APP_ASSERT_H

#include <app_configuration.h>
#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/**
 * @define DEFINE_THIS_FILE
 *
 * This macro must be defined in each file that wants to use any of the ASSERT
 * related macros.
 * When assertions are enabled it stores the filename for all assertions in that
 * specific file to keep only one copy of it to save FLASH memory.
 */

/**
 * @define ASSERT
 *
 * @param expr
 */

#if (CFG_APP_ASSERT_ENABLED == 0)

#define DEFINE_THIS_FILE_FOR_ASSERT
#define ASSERT(expr) ((void)0)

#else /* if CFG_APP_ASSERT_ENABLED == 0 */

#define DEFINE_THIS_FILE_FOR_ASSERT \
    static char const THIS_FILE__[] = __FILE__

#define ASSERT(expr) ((expr) ? (void)0 : AssertFailed(THIS_FILE__, __LINE__))


#endif /* if CFG_APP_ASSERT_ENABLED == 0 */


//#define ALLEGE(expr)    ASSERT(expr)
#define REQUIRE(expr)   ASSERT(expr)
#define ENSURE(expr)    ASSERT(expr)
#define INVARIANT(expr) ASSERT(expr)


/* ---------------------------------------------------------------------------
* Function prototype definitions
* --------------------------------------------------------------------------*/

/**
 *
 * @param p_file
 * @param line
 */
void AssertFailed(const char *p_file, int line);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* APP_ASSERT_H */
