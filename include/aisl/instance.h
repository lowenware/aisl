/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file aisl/instance.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Declarations of #AislInstance functions
 *
 * @see https://lowenware.com/aisl/

 */

#ifndef AISL_INSTANCE_H_60576F41_454C_4189_B91A_F40501132230
#define AISL_INSTANCE_H_60576F41_454C_4189_B91A_F40501132230

#include <stdint.h>
#include <stdarg.h>
#include <aisl/types.h>
#include <aisl/config.h>



/**
 * @brief Allocates new AISL instance.
 *
 * @param cfg a pointer to #aisl_cfg_t structure.
 * @return an #AislInstance instance pointer.
 */
AislInstance
aisl_new(const struct aisl_cfg *cfg);


/**
 * @brief Frees previously allocated pointer of AISL instance.
 * @param instance a pointer to #AislInstance instance.
 */
void
aisl_free(AislInstance instance);


/** 
 * @brief A core function doing all the library routines.
 * Designed to be called inside application main loop
 * @param instance a pointer to #AislInstance instance.
 * @return #AislStatus code.
 */
AislStatus
aisl_run_cycle(AislInstance instance);


/** 
 * @brief Function to sleep CPU if nothing to do.
 * Calls select on all the opened sockets inside.
 * @param instance a pointer to #AislInstance instance.
 * @param usec a number of miliseconds to wait for any data on sockets.
 * @return #AislStatus code.
 */
AislStatus
aisl_sleep(AislInstance instance, uint32_t usec);


#endif /* !AISL_INSTANCE_H */
