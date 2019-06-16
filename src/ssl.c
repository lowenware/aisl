/*
 * ssl.c
 * Copyright (C) 2019 Ilja Kartašov <ik@lowenware.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <openssl/err.h>
#include "str-utils.h"
#include "instance.h"
#include "ssl.h"


#if AISL_WITH_SSL == 1


static int
aisl_ssl_on_get_ctx(SSL *ssl, int *ptr, void *instance )
{
	const char *server_name = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
	SSL_CTX *ctx = aisl_get_ssl_ctx((AislInstance) instance, server_name);

	if (ctx)
		SSL_set_SSL_CTX(ssl, ctx);

	(void)ptr;

	return SSL_TLSEXT_ERR_OK;
}


SSL_CTX *
aisl_ssl_get_ctx(struct aisl_ssl *ssl, void *p_instance)
{
	SSL_CTX * ctx;

	if ((ctx = SSL_CTX_new(SSLv23_server_method())) != NULL) {
		SSL_CTX_set_ecdh_auto(ctx, 1);
		SSL_CTX_set_tlsext_servername_callback( ctx, aisl_ssl_on_get_ctx );
		SSL_CTX_set_tlsext_servername_arg(      ctx, p_instance );

		if (!(SSL_CTX_use_certificate_file(ctx, ssl->crt_file, SSL_FILETYPE_PEM)>0))
			goto except;

		if (!(SSL_CTX_use_PrivateKey_file(ctx, ssl->key_file, SSL_FILETYPE_PEM)>0))
			goto except;

		ssl->ctx = ctx;

		return ctx;
	}

except:
	SSL_CTX_free(ctx);
	return NULL;
}

struct aisl_ssl *
aisl_ssl_new(const char *key_file,
             const char *crt_file,
             const char *host,
             SSL_CTX    *ctx)
{
	struct aisl_ssl *ssl;

	if ((ssl = calloc(1, sizeof (struct aisl_ssl))) != NULL) {
		if ((ssl->host = str_copy( host ? host : "*" )) != NULL) {
			if (ctx) {
				ssl->ctx = ctx;
				return ssl;
			} else {
				if ((ssl->key_file = str_copy(key_file)) != NULL) {
					if ((ssl->crt_file = str_copy(crt_file)) != NULL) {
						return ssl;
					}
				}
			}
		}
		aisl_ssl_free(ssl);
	}
	return NULL;
}


void
aisl_ssl_free( struct aisl_ssl *ssl )
{
	if (ssl->host)
		free(ssl->host);

	if (ssl->key_file) {
		free(ssl->key_file);
		SSL_CTX_free(ssl->ctx);
	}

	if (ssl->crt_file)
		free(ssl->crt_file);

	free(ssl);
}

#endif
