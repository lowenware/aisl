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
 * src/ssl.h
 *
 * Copyright (C) 2019 Ilja Kartašov <ik@lowenware.com>
 *
 * Project homepage: https://lowenware.com/aisl/
 *
 */

#ifndef AISL_SSL_H_6F82B0BA_7C59_45BA_AF3B_C82A67C8585E
#define AISL_SSL_H_6F82B0BA_7C59_45BA_AF3B_C82A67C8585E

#include <aisl/config.h>
#include <aisl/types.h>
#include <openssl/ssl.h>


struct aisl_ssl {
	char    *key_file;
	char    *crt_file;
	char    *host;
	SSL_CTX *ctx;
};


struct aisl_ssl *
aisl_ssl_new( const char *key_file,
              const char *crt_file,
              const char *host,
              SSL_CTX    *ctx );


SSL_CTX *
aisl_ssl_get_ctx(struct aisl_ssl *ssl, void *p_instance);


void
aisl_ssl_free(struct aisl_ssl *ssl);


#endif /* !AISL_SSL_H */
