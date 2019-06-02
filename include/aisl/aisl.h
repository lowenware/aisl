/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file aisl/aisl.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Meta header file of AISL
 *
 * @see https://lowenware.com/aisl/
 */

#ifndef AISL_H_17EF1616_A00F_49C9_92B6_273AB13BF279
#define AISL_H_17EF1616_A00F_49C9_92B6_273AB13BF279

/* AISL configuration structure */
#include <aisl/config.h>

/* AISL types and stringifiers */
#include <aisl/types.h>

/* AISL instancing, initialization and processing */
#include <aisl/instance.h>

/* Embedded HTTP(s) servers */
#include <aisl/server.h>

/* HTTP(s) clients */
#include <aisl/client.h>

/* HTTP(s) streaming */
#include <aisl/stream.h>

#endif /* !AISL_H */
