/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file src/instance.h
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief Declarations of aisl_instance structure and functions
 *
 * @see https://lowenware.com/aisl/
 */

#ifndef AISL_INSTANCE_H_814CF474_A646_45B7_B6B2_3F4C7BEFA484
#define AISL_INSTANCE_H_814CF474_A646_45B7_B6B2_3F4C7BEFA484

#if AISL_WITH_SSL == 1
#include <openssl/ssl.h>
#endif

#include <aisl/instance.h>
#include "ssl.h"
#include "list.h"


struct aisl_instance {
	AislServer  *srv;
	#if AISL_WITH_SSL == 1
	struct aisl_ssl * *ssl;
	#endif
	struct list  client_spool;
	AislCallback callback;
	void        *p_ctx;

	uint32_t     accept_limit;
	uint32_t     silence_timeout;
	uint32_t     buffer_size;
};


#if AISL_WITH_SSL == 1
/**
 * @brief Gets SSL context for appropriate server name.
 * @param instance a pointer to #AislInstance instance.
 * @param server_name a null-terminated string with server name or NULL.
 * @return a pointer to SSL context
 */
SSL_CTX *
aisl_get_ssl_ctx(AislInstance instance, const char *server_name);
#endif


/** 
 * @brief Raises event from source.
 * @param instance a pointer to #AislInstance instance.
 * @param evt a pointer to event structure.
 */
void
aisl_raise_evt(AislInstance instance, const struct aisl_evt *evt);


void
aisl_raise(AislInstance  instance,
           void         *source,
           AislEvent     code,
           AislStatus    status);

#endif /* !AISL_INSTANCE_H */
