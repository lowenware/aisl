/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file debug.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief AISL debug module
 *
 * @see https://lowenware.com/
 */

#ifndef AISL_DEBUG_H_703E22F9_4A6E_426F_B708_81BCA019049B
#define AISL_DEBUG_H_703E22F9_4A6E_426F_B708_81BCA019049B

#if AISL_WITH_DEBUG == 1

#include <stdio.h>

#define DPRINTF(...) do {       \
  fprintf(stderr, "* AISL: ");    \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n");        \
} while(0)                      \


#else

#define DPRINTF(...)

#endif


#endif /* !AISL_DEBUG_H */
