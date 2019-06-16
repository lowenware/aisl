/*
 * src/instance.c
 *
 * Copyright (C) 2019 Ilja Kartašov <ik@lowenware.com>
 *
 * Project homepage: https://lowenware.com/aisl/
 *
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <sys/time.h>

#if AISL_WITH_SSL == 1
#include <openssl/err.h>
#endif

#include "debug.h"
#include "str-utils.h"
#include "buffer.h"
#include "client.h"
#include "server.h"
#include "stream.h"
#include "instance.h"


#if AISL_WITH_SSL == 1

static uint32_t m_instances = 0;

static struct aisl_ssl *
aisl_new_ssl(AislInstance instance, const struct aisl_cfg_ssl *cfg_ssl)
{
	SSL_CTX *ssl_ctx = NULL;
	struct aisl_ssl **list, *ssl;
	
	list = instance->ssl;

	/* lookup for existing contexts */
	while ((ssl = *list)) {
		if (ssl->key_file && strcmp(ssl->key_file, cfg_ssl->key_file)==0 &&
				ssl->crt_file && strcmp(ssl->crt_file, cfg_ssl->crt_file)==0
		) {
			ssl_ctx = ssl->ctx;
			break;
		}
		list++;
	}

	ssl = aisl_ssl_new(cfg_ssl->host, cfg_ssl->key_file, cfg_ssl->crt_file,
	  ssl_ctx);

	if (ssl) {
		if (!ssl_ctx && !aisl_ssl_get_ctx(ssl, (void*) instance)) {
			aisl_ssl_free(ssl);
			ssl = NULL;
		}
	}
	return ssl;
}

#endif


/* Initialization functions */

__attribute__ ((visibility ("default") ))
AislInstance
aisl_new(const struct aisl_cfg *cfg)
{
	int i;
	AislInstance instance;

	/* allocate root structure */
	if (!(instance = calloc(1, sizeof (struct aisl_instance))))
		goto finally;

	/* allocate servers */
	if (!(instance->srv = calloc(cfg->srv_cnt+1, sizeof (AislServer))))
		goto release;

	for (i = 0; i < cfg->srv_cnt; i++) {
		DPRINTF("new srv %d", i);
		if (!(instance->srv[i] = aisl_server_new(&cfg->srv[i], instance)))
			goto release;
	}

	#if AISL_WITH_SSL == 1
	if ((m_instances++) == 0) {
		SSL_load_error_strings();
		OpenSSL_add_ssl_algorithms();
	}

	if (!(instance->ssl = calloc(cfg->ssl_cnt+1, sizeof (struct aisl_ssl))))
		goto release;

	for (i=0; i<cfg->ssl_cnt; i++) {
		DPRINTF("new ssl %d", i);
		if (!(instance->ssl[i] = aisl_new_ssl(instance, &cfg->ssl[i])))
			goto release;
	}
	#endif

	if (list_init(&instance->client_spool, cfg->client_spool_size) == -1)
		goto release;

	instance->accept_limit = cfg->client_accept_limit;
	instance->silence_timeout = cfg->client_silence_timeout;
	instance->callback = cfg->callback;
	instance->p_ctx = cfg->p_ctx;

	goto finally;

release:
	aisl_free(instance);
	instance = NULL;

finally:
	return instance;
}


__attribute__ ((visibility ("default") ))
void
aisl_free(AislInstance instance)
{
	if (instance->srv) {
		AislServer * srv = instance->srv;

		while (*srv) {
			aisl_server_free(*(srv++));
		}

		free(instance->srv);
	}

	list_release(&instance->client_spool, (list_destructor_t)aisl_client_free);

	#if AISL_WITH_SSL == 1
	if (instance->ssl) {
		struct aisl_ssl **ssl = instance->ssl;

		while (*ssl) {
			aisl_ssl_free(*(ssl++));
		}
		free(instance->ssl);
	}

	if ((--m_instances) == 0) {
		EVP_cleanup();
	}
	#endif

	free(instance);
}


#if AISL_WITH_SSL == 1

SSL_CTX *
aisl_get_ssl_ctx(AislInstance instance, const char * host)
{
	struct aisl_ssl **list, *ssl;
	
	list = instance->ssl;

	if (host) {
		while ((ssl = *list)) {
			if (str_cmpi(ssl->host, host) == 0) {
				return ssl->ctx;
			}
			list++;
		}
	}
	return NULL;
}

#endif


void
aisl_raise_evt(AislInstance instance, const struct aisl_evt *evt)
{
	#if AISL_WITH_STRINGIFIERS == 1
	DPRINTF("! %s", aisl_event_to_string(evt->code));
	#else
	DPRINTF("! %d", evt->code);
	#endif

	if (instance->callback)
		instance->callback(evt, instance->p_ctx);
}


void
aisl_raise(AislInstance  instance,
           void         *source,
           AislEvent     code,
           AislStatus    status)
{
	struct aisl_evt evt;

	evt.source = source;
	evt.code   = code;
	evt.status = status;

	aisl_raise_evt(instance, &evt);
}


__attribute__ ((visibility ("default") ))
AislStatus
aisl_run_cycle(AislInstance instance)
{
	AislStatus result = AISL_IDLE;
	AislServer *list, srv;
	AislClient cli;
	int32_t i;

	list = instance->srv;

	while ((srv = *list)) {
		cli = NULL;

		if (aisl_server_touch(srv, &cli) != AISL_IDLE)
			result = AISL_SUCCESS;

		if (cli) {
			DPRINTF("Accepted %p", (void*)cli);
			if (list_append(&instance->client_spool, cli) == -1)
				aisl_client_free(cli);
		}
		list++;
	}

	for (i=0; i < instance->client_spool.count; i++) {
		cli = LIST_INDEX(instance->client_spool, i);

		if (aisl_client_touch(cli, instance->silence_timeout) != AISL_IDLE)
			result = AISL_SUCCESS;

		if (!aisl_client_is_online(cli)) {
			aisl_client_free( cli );
			list_remove_index(&instance->client_spool, i);
		}
	}
	return result;
}


__attribute__ ((visibility ("default") ))
AislStatus
aisl_sleep(AislInstance instance, uint32_t usec)
{
	AislServer *list, srv;
	int sd, maxfd = 0;
	size_t i;
	struct timeval timeout = {0,usec};

	memset(&timeout, 0, sizeof (struct timeval));
	timeout.tv_usec = usec;

	fd_set  fs;
	FD_ZERO (&fs);

	list = instance->srv;

	while ((srv = *list)) {
		sd = aisl_server_get_socket(srv);

		if (sd != -1) {
			FD_SET(sd, &fs);
			if (sd > maxfd) maxfd = sd;
		}
		list++;
	}

	for (i=0; i<instance->client_spool.count; i++) {
		AislClient c = LIST_INDEX(instance->client_spool, i);
		sd = aisl_client_get_socket(c);
		if (sd != -1) {
			FD_SET(sd, &fs);
			if (sd > maxfd) maxfd = sd;
		}
	}

	switch (select(maxfd+1, &fs, NULL, NULL, &timeout)) {
	case -1:
		return AISL_SYSCALL_ERROR;

	case 0:
		return AISL_IDLE;

	default:
		return AISL_SUCCESS;
	}
}
