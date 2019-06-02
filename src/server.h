/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file dummy.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief 
 *
 * @see https://lowenware.com/
 *//*
 * @file src/server.h
 *
 * Copyright (c) 2017-2019 by Löwenware Ltd.
 *
 * Project homepage: https://lowenware.com/aisl/
 *
 */

#ifndef AISL_SERVER_H_FACD3B4D_0D72_4345_9A30_811103DA9AE2
#define AISL_SERVER_H_FACD3B4D_0D72_4345_9A30_811103DA9AE2

#include <aisl/config.h>
#include <aisl/server.h>

#define AISL_SERVER(x) ((AislServer) x)

/**
 * @brief HTTP(s) server data structure represented by #AislServer pointer.
 */
struct aisl_server {
	struct sockaddr_in   address;  /**< TCP server address to listen to. */
	AislInstance         instance; /**< Associated AISL instance pointer. */
	int                  fd;       /**< System socket descriptor. */
	bool                 ssl;      /**< SSL enabled/disabled flag. */
};

/**
 * @brief Allocates and instance of #AislServer.
 * @param cfg_srv a pointer to server configuration structure.
 * @param instance a pointer to #AislInstance instance.
 * @return a pointer to #AislServer instance.
 */
AislServer
aisl_server_new(const struct aisl_cfg_srv *cfg_srv, AislInstance instance);


/**
 * @brief Frees memory allocated for #AislServer instance.
 * @param server a pointer to #AislServer instance.
 */
void
aisl_server_free(AislServer server);


/**
 * @brief Does server routines.
 * Tries to open server if it was not opened yet, otherwise tries to accept a
 * new client connecting to the server.
 * @param server a pointer to #AislServer instance.
 * @param p_client a pointer to store #AislClient instance pointer.
 * @return #AislStatus code:
 * - AISL_SUCCESS if client connected,
 * - AISL_IDLE if there is no client to connect,
 * - AISL_SYSCALL_ERROR if error occured.
 */
AislStatus
aisl_server_touch(AislServer  server, AislClient *p_client);


/**
 * @brief Gets a socket descriptor associated with HTTP client.
 * @param server a pointer to #AislServer instance.
 * @return a client socket descriptor.
 */
int
aisl_server_get_socket(AislServer server);


#endif /* !AISL_SERVER_H */
