#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#if AISL_WITH_SSL == 1
#include <openssl/err.h>
#endif

#include <aisl/aisl.h>
#include "debug.h"
#include "stream.h"
#include "http.h"
#include "server.h"
#include "instance.h"
#include "client.h"

#define FLAG_KEEPALIVE (1<<0)
#define FLAG_HANDSHAKE (1<<1)
#define FLAG_CAN_READ  (1<<2)
#define FLAG_CAN_WRITE (1<<3)

#define BUFFER_SIZE (16*1024)


static void
aisl_client_close(AislClient client, AislStatus status)
{
	if (client->fd != -1) {
		aisl_raise(client->server->instance, (void *)client,
		  AISL_EVENT_CLIENT_DISCONNECT, status );

		close(client->fd);
		shutdown(client->fd, SHUT_RDWR);
		client->fd = -1;
	}
}


static AislStatus
aisl_client_parse(AislClient client, char *data, int32_t size)
{
	AislStatus result = AISL_SUCCESS;
	AislStream s = client->stream;
	ParserStatus p = HTTP_PARSER_SUCCESS;

	int32_t data_size;

	switch (client->http_version) {
	case AISL_HTTP_0_9:
	case AISL_HTTP_1_0:
	case AISL_HTTP_1_1:
		while (p == HTTP_PARSER_SUCCESS) {
			data_size = size;

			switch (aisl_stream_get_state(s)) {
			case AISL_STREAM_STATE_IDLE:
				p = http_10_parse_request(data, &size, client->stream);
				break;

			case AISL_STREAM_STATE_WAIT_HEADER:
				p = http_10_parse_header(data, &size, client->stream);
				break;

			case AISL_STREAM_STATE_WAIT_BODY:
				p = http_10_parse_body(data, &size, client->stream);
				break;

			default: /* has input data, but request was already parsed */
				DPRINTF("misleading request length");
				p = HTTP_PARSER_ERROR;
				continue;
			}
			// size now has number of parsed bytes
			DPRINTF("%d bytes parsed, %d bytes left", (data_size - size), size);
			data += (data_size - size);
		}
		break;

	case AISL_HTTP_2_0:
		break;
	}

	switch(p) {
	case HTTP_PARSER_READY:
		client->flags &= ~FLAG_CAN_READ;
		client->flags |= FLAG_CAN_WRITE;
		client->stream->content_length = AISL_AUTO_LENGTH;
		aisl_raise(client->server->instance, (void *)s, AISL_EVENT_STREAM_REQUEST,
		  result);
		break;

	case HTTP_PARSER_ERROR:
		/* reply Bad Request here */
		client->stream->http_response = AISL_HTTP_BAD_REQUEST;
		aisl_raise(client->server->instance, (void *)s, AISL_EVENT_STREAM_ERROR,
		  result);

		aisl_client_close(client, result);
		return result;

	default:
		break;
	}

	buffer_shift(&client->in, client->in.used - size); /* reset buffer */

	return result;
}


/* In HTTP 2.0 client->stream will be NULL if stream related data was completely
 * parsed. If it is not NULL, then stream expects additional data -> same like
 * in mono stream HTTP 1.0 
 */
static AislStatus
aisl_client_input(AislClient client)
{
	int l;
	char *data = &client->in.data[ client->in.used ];
	int32_t size = client->in.size - client->in.used;

	#if AISL_WITH_SSL == 1
	if (client->ssl) {
		DPRINTF("SSL_read");
		if (!(client->flags & FLAG_HANDSHAKE)) {
			if ( (l = SSL_accept(client->ssl)) != 1 ) {
				l = SSL_get_error(client->ssl, l);

				if (l == SSL_ERROR_WANT_READ || l == SSL_ERROR_WANT_WRITE)
					return AISL_IDLE;

				DPRINTF("SSL handshake fail: %s\n", ERR_error_string(l, NULL) );

				aisl_client_close(client, AISL_EXTCALL_ERROR);
				return AISL_EXTCALL_ERROR;
			}
			client->flags &= ~FLAG_HANDSHAKE;
		}
		l = SSL_read(client->ssl, data, size);
	} else
	#endif
	{
		l = recv( client->fd, data, size, 0);
	}

	if (l > 0) {
		DPRINTF("%d bytes received from client", l);

		data = client->in.data;
		size = client->in.used + l;
		client->in.used = size;
		return aisl_client_parse(client, data, size);
	} else if (l<0) {

		#if AISL_WITH_SSL == 1
		if (client->ssl) {
			if (SSL_get_error(client->ssl, l) == SSL_ERROR_WANT_READ)
				return AISL_IDLE;
		} else
		#endif
		{

			if(errno == EWOULDBLOCK)
				return AISL_IDLE;
			DPRINTF("client - %s", strerror(errno));
		}
	}

	/* both: client disconnect + on read error  */
	/* todo: raise client error here */
	aisl_client_close(client, AISL_SYSCALL_ERROR);

	return AISL_SYSCALL_ERROR;
}


static AislStatus 
aisl_client_output(AislClient client)
{
	int l;
	char *data;

	AislStream s = client->stream;

	/* while stream is not flushed, we should raise event */
	if(aisl_get_output_event(s)) {
		/* in case of chunked output ( subscription for AISL_STREAM_OUTPUT event )
		 * stream buffer will be initialized with OUTPUT_BUFFER_SIZE size, but
		 * buffer->size will be used to carry amount of stored bytes
		 * */
		l = aisl_stream_get_buffer_space(s);

		/*
		if (bsz < OUTPUT_BUFFER_SIZE)
		{
			if (buffer_clear(s->buffer, OUTPUT_BUFFER_SIZE) == 0)
				return false;

			s->buffer->size = bsz;
			bsz = OUTPUT_BUFFER_SIZE;
		}
		*/

		if (!(l < aisl_stream_get_buffer_size(s) / 2)) {
			aisl_raise(client->server->instance, (void*)s, AISL_EVENT_STREAM_OUTPUT,
			  AISL_SUCCESS);
		}
	}

	data = aisl_stream_get_data(s, &l);

	if ( !l )
		return AISL_IDLE;

	#if AISL_WITH_SSL == 1
	l = (client->ssl) ?  SSL_write(client->ssl, data, l) :
		send(client->fd,  data, l, 0);
	#else
	l = send(client->fd,  data, l, 0);
	#endif

	if (l > 0) {
		aisl_stream_shift(s, l);
		if (aisl_stream_is_done(s)) {
			/* data has been sent */
			if (client->flags & FLAG_KEEPALIVE) {
				aisl_stream_free(s);

				client->stream = aisl_stream_new(client, client->next_id++);
				if (client->stream != NULL )
					return AISL_SUCCESS;

				/* in case of malloc error it will not be an error as long as the 
				 * request was handled and we just close the connection.
				 */
			}

			aisl_client_close(client, AISL_SUCCESS);
		}

		return AISL_SUCCESS;
	}

	/* l < 0 */
	#if AISL_WITH_SSL == 1
	if (client->ssl) {
		if (SSL_get_error(client->ssl, l) == SSL_ERROR_WANT_WRITE)
			return AISL_IDLE;
	} else
	#endif
	{
		if (errno == EWOULDBLOCK)
			return AISL_IDLE;
	}
	aisl_client_close(client, AISL_SYSCALL_ERROR);

	return AISL_SYSCALL_ERROR;
}


AislClient
aisl_client_new(AislServer server, int fd, struct sockaddr_in *addr)
{
	AislClient   client;
	AislStream   stream;

	if ((client = calloc(1, sizeof (struct aisl_client))) != NULL) {
		DPRINTF("client alocated");
		memcpy(&client->address, addr, sizeof (struct sockaddr_in));
		client->server     = server;
		client->fd         = fd;
		client->next_id    = 2;
		client->http_version = AISL_HTTP_1_0;
		client->timestamp  = time(NULL);
		client->flags      = FLAG_KEEPALIVE | FLAG_HANDSHAKE | FLAG_CAN_READ;

		if (buffer_init(&client->in, 2*BUFFER_SIZE) != -1) {
			DPRINTF("client buffer alocated");
			memcpy(&client->out, &client->in, sizeof (struct buffer));

			stream = aisl_stream_new(client, 0);

			if (stream != NULL) {
				client->stream = stream;
				DPRINTF("client stream alocated");

				#if AISL_WITH_SSL == 1
				if (server->ssl) {
					SSL_CTX * ssl_ctx = aisl_get_ssl_ctx(server->instance, NULL);

					if ((client->ssl = SSL_new(ssl_ctx)) != NULL ) {
						SSL_set_fd(client->ssl, fd);
						return client;
					}
				} else {
					return client;
				}
				#else
				return client;
				#endif
			}
		}
		aisl_client_free(client);
	}
	return NULL;
}


void
aisl_client_free(AislClient client)
{
	aisl_client_close(client, AISL_SUCCESS);

	#if AISL_WITH_SSL == 1
	if (client->ssl)
		SSL_free(client->ssl);
	#endif

	if (client->in.data)
		free(client->in.data);

	if (client->stream)
		aisl_stream_free(client->stream);

	free(client);
}


AislStatus
aisl_client_touch(AislClient client, int32_t timeout)
{
	AislStatus result, status;
	
	result = AISL_IDLE;
	status = AISL_IDLE;

	/* input */
	if (client->flags & FLAG_CAN_READ) {
		if ( (result = aisl_client_input(client)) < 0 )
			return result;
	}

	/* output */
	if (client->flags & FLAG_CAN_WRITE) {
		if ( (status = aisl_client_output(client)) < 0 )
			return status;
	}

	if (result == AISL_IDLE)
		result = status;

	if (result != AISL_SUCCESS) {
		time_t now;
		time(&now);

		if (!(now - client->timestamp < timeout)) {
			aisl_client_close(client, result);
		}
	} else {
		client->timestamp = time(NULL);
	}
	return result;
}


int
aisl_client_get_socket(AislClient client)
{
	return client->fd;
}


bool
aisl_client_get_keepalive(AislClient client)
{
	return (client->flags & FLAG_KEEPALIVE);
}


void
aisl_client_set_keepalive(AislClient client, bool value)
{
	if (value)
		client->flags |= FLAG_KEEPALIVE;
	else
		client->flags &= ~FLAG_KEEPALIVE;
}



/* API Level ---------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
AislServer
aisl_client_get_server(AislClient client)
{
	return client->server;
}


__attribute__ ((visibility ("default") ))
bool
aisl_client_is_secure(AislClient client)
{
	#if AISL_WITH_SSL == 1
	return (client->ssl == NULL) ? false : true;
	#else
	return false;
	#endif
}


__attribute__ ((visibility ("default") ))
bool
aisl_client_is_online(AislClient client)
{
	return (client->fd == -1) ? false : true;
}


__attribute__ ((visibility ("default") ))
void
aisl_client_disconnect(AislClient client)
{
	aisl_client_close(client, AISL_SUCCESS);
}


__attribute__ ((visibility ("default") ))
AislHttpVersion
aisl_client_get_http_version(AislClient client)
{
	return client->http_version;
}


__attribute__ ((visibility ("default") ))
void
aisl_client_get_address(AislClient client, struct sockaddr_in *address)
{
	memcpy(address, &client->address, sizeof (struct sockaddr_in));
}
