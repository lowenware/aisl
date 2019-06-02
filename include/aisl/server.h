/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file aisl/server.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Declarations of #AislServer functions
 *
 * @see https://lowenware.com/aisl/
 */

#ifndef AISL_SERVER_H_CC564608_7A05_4B31_9E7E_32750BC60768
#define AISL_SERVER_H_CC564608_7A05_4B31_9E7E_32750BC60768

#include <arpa/inet.h>
#include <aisl/types.h>

/**
 * @brief Function to get appropriate AISL instance pointer from server pointer.
 * @param server an #AislServer pointer.
 * @return an #AislInstance instance pointer.
 */
AislInstance
aisl_server_get_instance(AislServer server);


/**
 * @brief Copies server listen address information to sockaddr_in structure.
 * @param server an #AislServer pointer.
 * @param address a pointer to sockaddr_in structure.
 */
void
aisl_server_get_address(AislServer server, struct sockaddr_in *address);


/**
 * @brief Function to get on and off status of SSL for the #AislServer.
 * @param server an #AislServer pointer.
 * @return a boolean value representing SSL enabled/disabled state.
 */
bool
aisl_server_get_ssl(AislServer server);

#endif /* !AISL_SERVER_H */
