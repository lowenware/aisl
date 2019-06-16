/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file client.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Declarations of aisl_client structure and functions
 *
 * @see https://lowenware.com/aisl/
 */
#ifndef AISL_CLIENT_H_164FE6B2_E5D4_4968_B50F_823E30E8F777
#define AISL_CLIENT_H_164FE6B2_E5D4_4968_B50F_823E30E8F777

#include <arpa/inet.h>

#if AISL_WITH_SSL == 1
#include <openssl/ssl.h>
#endif

#include <aisl/client.h>
#include "buffer.h"

#define AISL_CLIENT(x) ((AislClient) x)


struct aisl_client {
	struct sockaddr_in  address;       /**< Client's address structure. */
	struct buffer       in;            /**< Client's input buffer. */
	struct buffer       out;           /**< Client's output buffer. */
	AislServer          server;        /**< Server instance. */
	AislStream          stream;        /**< Pending client's stream. */
	#if AISL_WITH_SSL == 1
	SSL                *ssl;           /**< SSL pointer for HTTPS. */
	#endif
	time_t              timestamp;     /**< Last communication timestamp. */
	int                 next_id;       /**< Stream id generator (even). */
	int                 flags;         /**< Client's flag bitmask. */
	int                 fd;            /**< Client's socket descriptor. */
	AislHttpVersion     http_version;  /**< Client's http_version version. */
};


/**
 * @brief Constructor for #AislClient instance.
 * @param server an #AislServer instance pointer.
 * @param fd a client socket descriptor.
 * @param addr a pointer to client's address structure.
 * @param ssl_ctx a pointer to SSL context or NULL if encryption is disabled
 */
AislClient
aisl_client_new(AislServer          server,
                int                 fd,
                struct sockaddr_in *addr );


/**
 * @brief Destructor for #AislClient instance.
 * @param client an #AislClient instance pointer.
 */
void
aisl_client_free(AislClient client);


/**
 * @brief Does all HTTP client routines.
 * Reads and parses requests, writes responses.
 * @param client an #AislClient instance pointer.
 * @param timeout an allowed client silence time in seconds.
 * @return #AislStatus code.
 */
AislStatus
aisl_client_touch(AislClient client, int32_t timeout);


/**
 * @Brief Checks if client is about to keep connection alive.
 * @param client an #AislClient instance pointer.
 * @return true if keepalive mode is on, otherwise false.
 */
bool
aisl_client_get_keepalive(AislClient client);


/**
 * @Brief Sets if connection with client must be kept alive.
 * @param client an #AislClient instance pointer.
 * @param value a true to enable keepalive mode, false to disable.
 */
void
aisl_client_set_keepalive(AislClient client, bool value);


/**
 * @brief Gets socket descriptor associated with #AislClient instance.
 * @param client an #AislClient instance pointer.
 * @return a client socket descriptor.
 */
int
aisl_client_get_socket(AislClient client);


#endif /* !AISL_CLIENT_H */
