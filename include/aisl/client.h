/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file aisl/client.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Declarations of #AislCLient functions
 *
 * @see https://lowenware.com/aisl/
 */

#ifndef AISL_CLIENT_H_A6C37DCF_2183_4F22_A5A0_668311757A08
#define AISL_CLIENT_H_A6C37DCF_2183_4F22_A5A0_668311757A08

#include <aisl/types.h>


/**
 * @brief Gets #AislServer instance associated with client.
 * @param client an #AislClient instance pointer.
 * @return an associated #AislServer pointer.
 */
AislServer
aisl_client_get_server(AislClient client);


/**
 * @brief Gets security connection status.
 * @param client an #AislClient instance pointer.
 * @return true if SSL is enabled and false if disabled.
 */
bool
aisl_client_is_secure(AislClient client);


/**
 * @brief Gets client's connection state.
 * @param client an #AislClient instance pointer.
 * @return true if client is online and false if is offline.
 */
bool
aisl_client_is_online(AislClient client);


/**
 * @brief Forcefully closes client's connection.
 * @param client an #AislClient instance pointer.
 */
void
aisl_client_disconnect(AislClient client);


/**
 * @brief Gets HTTP protocol version.
 * @param client an #AislClient instance pointer.
 * @return HTTP protocol version
 */
AislHttpVersion
aisl_client_get_http_version(AislClient client);


/**
 * @brief Copies #AislClient network address to provided sockaddr_in structure.
 * @param client an #AislClient instance pointer.
 * @param address a pointer to a sockaddr_in structure
 */
void
aisl_client_get_address(AislClient client, struct sockaddr_in *address);

#endif /* !AISL_CLIENT_H */
