#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

#include "instance.h"
#include "client.h"
#include "server.h"
#include "str-utils.h"
#include "debug.h"
#include "stream.h"

#define FLAG_FLUSHED                    (1<<0)
#define FLAG_OUTPUT_CHUNKED             (1<<1)
#define FLAG_SERVER_HEADER_SENT         (1<<2)
#define FLAG_CONTENT_TYPE_HEADER_SENT   (1<<3)
#define FLAG_CONTENT_LENGTH_HEADER_SENT (1<<4)
#define FLAG_CONNECTION_HEADER_SENT     (1<<5)


/* Library level */

static void
aisl_stream_reset(AislStream stream, bool initial)
{
	if (!initial) {
		aisl_raise(aisl_get_instance(stream), (void*) stream,
		  AISL_EVENT_STREAM_CLOSE, AISL_SUCCESS);
	}

	buffer_release(&stream->buffer);

	stream->u_ptr          = NULL;
	stream->content_length = 0; //AISL_AUTO_LENGTH;
	stream->head_offset    = 0;
	stream->flags          = 0;
	stream->state          = AISL_STREAM_STATE_IDLE;
	stream->http_response  = AISL_HTTP_OK;
}


AislStream
aisl_stream_new(AislClient client, int id)
{
	AislStream stream = calloc(1, sizeof (struct aisl_stream));

	if (stream) {
		stream->id     = id;
		stream->client = client;
		aisl_stream_reset(stream, true);
	}
	return stream;
}


void
aisl_stream_free(AislStream stream)
{
	aisl_stream_reset(stream, false);
	free(stream);
}


int32_t
aisl_stream_get_buffer_space(AislStream stream)
{
	return stream->buffer.size - stream->buffer.used;
}


int32_t
aisl_stream_get_buffer_size(AislStream stream)
{
	return stream->buffer.size;
}


char *
aisl_stream_get_data(AislStream stream, int32_t *p_length)
{
	*p_length = stream->buffer.used;

	return stream->buffer.data;
}


void
aisl_stream_shift(AislStream stream, int32_t offset)
{
	buffer_shift(&stream->buffer, offset);
}


bool
aisl_stream_is_done(AislStream stream)
{
	return (!stream->buffer.used && stream->state == AISL_STREAM_STATE_DONE);
}


AislStreamState
aisl_stream_get_state(AislStream stream)
{
	return stream->state;
}


void
aisl_stream_set_request(AislStream      stream,
                        AislHttpMethod  http_method,
                        const char     *path,
                        const char     *query)
{
	struct aisl_evt_open on_open;

	stream->state = AISL_STREAM_STATE_WAIT_HEADER;

	DPRINTF("%s -> path: %s, query: %s", aisl_http_method_to_string(http_method),
	  path, query);

	on_open.evt.code    = AISL_EVENT_STREAM_OPEN;
	on_open.evt.source  = (void *) stream;
	on_open.evt.status  = AISL_SUCCESS;
	on_open.http_method = http_method;
	on_open.path        = path;
	on_open.query       = query;

	aisl_raise_evt(aisl_get_instance(stream), (struct aisl_evt *)&on_open);
}


void
aisl_stream_set_header(AislStream stream, const char *key, const char *value)
{
	struct aisl_evt_header on_header;

	if (stream->state != AISL_STREAM_STATE_WAIT_HEADER)
		return;

	if (strcmp(key, "content-length") == 0) {
		stream->content_length = strtoll(value, NULL, 10);
	} else if (strcmp(key, "connection") == 0) {
		aisl_client_set_keepalive(stream->client,
		  (str_cmpi(value, "close")==0) ? false : true);
	}

	DPRINTF("%s: %s", key, value);

	on_header.evt.code   = AISL_EVENT_STREAM_HEADER;
	on_header.evt.source = (void *) stream;
	on_header.evt.status = AISL_SUCCESS;
	on_header.key        = key;
	on_header.value      = value;

	aisl_raise_evt(aisl_get_instance(stream),
	  (struct aisl_evt *) &on_header);
}


int
aisl_stream_set_end_of_headers(AislStream stream)
{
	int result;

	DPRINTF("stream->content_length == %"PRIu64"", stream->content_length);

	if (stream->state == AISL_STREAM_STATE_WAIT_HEADER) {
		result = (stream->content_length != 0);
		stream->state = (result) ? AISL_STREAM_STATE_WAIT_BODY :
			AISL_STREAM_STATE_READY;
	} else {
		result = 2;
	}

	return result;
}


int
aisl_stream_set_body(AislStream stream, const char *data, int32_t size)
{
	int result;
	if (stream->state == AISL_STREAM_STATE_WAIT_BODY) {
		if (!(stream->content_length < size)) {
			struct aisl_evt_input on_input;

			stream->content_length -= size;

			if (stream->content_length == 0) {
				stream->state = AISL_STREAM_STATE_READY;
				result = 0;
			} else {
				result = 1;
			}

			on_input.evt.code   = AISL_EVENT_STREAM_INPUT;
			on_input.evt.source = (void *)stream;
			on_input.evt.status = AISL_SUCCESS;
			on_input.data       = data;
			on_input.size       = size;

			aisl_raise_evt(stream->client->server->instance,
			  (struct aisl_evt *) &on_input);
		} else {
			result = -1;
		}
	} else {
		result = 2;
	}
	return result;
}

/* API Level */

/* Why it was here?
static int
aisl_stream_write(AislStream stream, const char * data, uint32_t d_len)
{
	return buffer_add( &stream->buffer, data, d_len);
}


__attribute__ ((visibility ("default") ))
void
aisl_cancel(AislStream stream)
{
	aisl_client_close( stream->client );
}
*/

__attribute__ ((visibility ("default") ))
void *
aisl_get_context(AislStream s)
{
	return s->u_ptr;
}


__attribute__ ((visibility ("default") ))
void
aisl_set_context(AislStream s, void *u_ptr)
{
	s->u_ptr = u_ptr;
}


__attribute__ ((visibility ("default") ))
bool
aisl_is_secure(AislStream stream)
{
	return aisl_client_is_secure(stream->client);
}


__attribute__ ((visibility ("default") ))
AislClient
aisl_get_client(AislStream s)
{
	return s->client;
}


__attribute__ ((visibility ("default") ))
AislServer
aisl_get_server(AislStream s)
{
	return aisl_client_get_server(s->client);
}


__attribute__ ((visibility ("default") ))
AislHttpVersion
aisl_get_http_version(AislStream s)
{
	return aisl_client_get_http_version(s->client);
}


__attribute__ ((visibility ("default") ))
void
aisl_reject(AislStream s)
{
	aisl_client_disconnect( s->client );
}


static AislStatus
aisl_start_response(AislStream stream)
{
	return aisl_response(stream, AISL_HTTP_OK, AISL_AUTO_LENGTH);
}


static AislStatus
aisl_stream_close_headers(AislStream stream)
{
	int32_t l;

	if (aisl_start_response(stream) == AISL_MALLOC_ERROR)
		return AISL_MALLOC_ERROR;

	if (!(stream->flags & FLAG_SERVER_HEADER_SENT)) {
		l = buffer_append(&stream->buffer, "Server: AISL\r\n", 14);
		if (l == -1)
			return AISL_MALLOC_ERROR;

		stream->flags |= FLAG_SERVER_HEADER_SENT;
	}

	if (!(stream->flags & FLAG_CONTENT_TYPE_HEADER_SENT)) {
		l = buffer_append(&stream->buffer,
		  "Content-Type: text/html; encoding=utf-8\r\n", 41);

		if (l == -1)
			return AISL_MALLOC_ERROR;

		stream->flags |= FLAG_CONTENT_TYPE_HEADER_SENT;
	}

	if (!(stream->flags & FLAG_CONTENT_LENGTH_HEADER_SENT)) {
		if (stream->content_length != AISL_AUTO_LENGTH) {
			l = buffer_append_printf(&stream->buffer, "Content-Length: %"PRIu64"\r\n",
			  stream->content_length);

			if (l == -1)
				return AISL_MALLOC_ERROR;

			stream->flags |= FLAG_CONTENT_LENGTH_HEADER_SENT;
		}
	}

	if (!(stream->flags & FLAG_CONNECTION_HEADER_SENT)) {
		l = buffer_append_printf(&stream->buffer, "Connection: %s\r\n",
		  (aisl_client_get_keepalive(stream->client) ? "keepalive" : "close"));

		if (l == -1)
			return AISL_MALLOC_ERROR;

		stream->flags |= FLAG_CONNECTION_HEADER_SENT;
	}

	if (buffer_append( &stream->buffer, "\r\n", 2 ) == -1)
		return AISL_MALLOC_ERROR;

	stream->body_offset = stream->buffer.used;
	stream->state = AISL_STREAM_STATE_SEND_BODY;

	return AISL_SUCCESS;
}


__attribute__ ((visibility ("default") ))
AislStatus
aisl_response(AislStream stream, AislHttpResponse rs_code, uint64_t c_len)
{
	int32_t l;

	/* check if those headers were already sent */
	if (stream->state > AISL_STREAM_STATE_READY)
		return AISL_IDLE;

	stream->http_response  = rs_code;
	stream->content_length = c_len;

	buffer_init(&stream->buffer, (c_len != AISL_AUTO_LENGTH) ? c_len : 0);

	l = buffer_append_printf(&stream->buffer, "%s %d %s\r\n",
	  aisl_http_version_to_string(stream->client->http_version), rs_code,
	  aisl_http_response_to_string(rs_code));

	if (l == -1)
		return AISL_MALLOC_ERROR;

	stream->state = AISL_STREAM_STATE_SEND_HEADER;

	return AISL_SUCCESS;
}


__attribute__ ((visibility ("default") ))
AislStatus
aisl_flush(AislStream s)
{
	if (!(s->flags & FLAG_CONTENT_LENGTH_HEADER_SENT)) {
		char hdr[ 40 ];
		uint64_t c_len;
		int32_t  l;

		if (s->body_offset) {
			c_len = s->buffer.used - s->body_offset;
			l = snprintf(hdr, sizeof (hdr), "Content-Length: %"PRIu64"\r\n", c_len);
			l = buffer_insert(&s->buffer, s->body_offset - 2, hdr, l);
			if (l == -1)
				return AISL_MALLOC_ERROR;
		} else {
			aisl_stream_close_headers(s);
		}
		s->flags |= FLAG_CONTENT_LENGTH_HEADER_SENT;
	}

	s->state = AISL_STREAM_STATE_DONE;
	s->flags |= FLAG_FLUSHED;

	return AISL_SUCCESS;
}


static int32_t
aisl_stream_verify_header(AislStream stream, const char *key, const char *value)
{
	if (stream->state < AISL_STREAM_STATE_SEND_HEADER) {
		if (aisl_start_response(stream) != AISL_SUCCESS)
			return -1;
	} else if (stream->state > AISL_STREAM_STATE_SEND_HEADER) {
		return 0;
	}

	if (!(stream->flags & FLAG_CONNECTION_HEADER_SENT)) {
		if (str_cmpi(key, "connection")==0) {
			stream->flags |= FLAG_CONNECTION_HEADER_SENT;
			if (value) {
				aisl_client_set_keepalive(stream->client,
				  (str_cmpi(value, "keepalive") == 0));
			}
			return 1;
		}
	}

	if (!(stream->flags & FLAG_CONTENT_TYPE_HEADER_SENT)) {
		if (str_cmpi(key, "content-type") == 0) {
			stream->flags |= FLAG_CONTENT_TYPE_HEADER_SENT;
			return 1;
		}
	}

	if (!(stream->flags & FLAG_CONTENT_LENGTH_HEADER_SENT)) {
		if (str_cmpi(key, "content-length") == 0) {
			stream->flags |= FLAG_CONTENT_LENGTH_HEADER_SENT;
			return 1;
		}
	}

	if (!(stream->flags & FLAG_CONTENT_LENGTH_HEADER_SENT)) {
		if (str_cmpi(key, "content-length")==0) {
			stream->flags |= FLAG_CONTENT_LENGTH_HEADER_SENT;
			return 1;
		}
	}

	return 1;
}

__attribute__ ((visibility ("default") ))
int32_t
aisl_header(AislStream stream, const char *key, const char *value)
{
	int32_t result;

	if ( (result = aisl_stream_verify_header( stream, key, value )) != 1)
		return result;

	result = buffer_append_printf(&stream->buffer, "%s: %s\r\n", key, value);

	return result;

	/* For debug purposes
	if ( (pch = str_printf("%s: %s\r\n", key, value)) != NULL )
	{
		ret = strlen(pch);
		if ( buffer_insert(
					 &stream->buffer,
					 stream->end_of_headers,
					 pch,
					 ret
				 ) == -1 )
		{
			ret = -1;
		}
		else
			stream->end_of_headers += ret;

		free(pch);
	}
	else
		ret = -1;

	return ret;
	*/
}


__attribute__ ((visibility ("default") ))
int32_t
aisl_header_printf(AislStream stream, const char *key, const char *format, ...)
{
	int32_t result;
	va_list args;

	va_start(args, format);
	result = aisl_header_vprintf( stream, key, format, args );
	va_end(args);

	return result;
}


__attribute__ ((visibility ("default") ))
int32_t
aisl_header_vprintf(AislStream  stream,
                    const char *key,
                    const char *format,
                    va_list     args)
{
	int32_t result, l;

	if ( (result = aisl_stream_verify_header( stream, key, NULL )) != 1)
		return result;

	result = buffer_append_printf( &stream->buffer, "%s: ", key );

	if (result != -1) {
		l = buffer_append_vprintf( &stream->buffer, format, args );

		if (l != -1) {
			result += l;
			if ((l = buffer_append(&stream->buffer, "\r\n", 2)) != -1) {
				result += l;
				return result;
			}
		}
	}
	return -1;
}


__attribute__ ((visibility ("default") ))
int
aisl_printf(AislStream stream, const char *format, ...)
{
	int result;
	va_list arg;

	va_start(arg, format);
	result = aisl_vprintf(stream, format, arg);
	va_end(arg);

	return result;
}


__attribute__ ((visibility ("default") ))
int32_t
aisl_vprintf(AislStream stream, const char *format, va_list args)
{
	if (stream->state < AISL_STREAM_STATE_SEND_BODY) {
		if (aisl_stream_close_headers(stream) != AISL_SUCCESS)
			return -1;
	}
	return buffer_append_vprintf(&stream->buffer, format, args);
}


__attribute__ ((visibility ("default") ))
int32_t
aisl_write(AislStream stream, const char *data, int32_t d_len)
{
	if (stream->state < AISL_STREAM_STATE_SEND_BODY) {
		if (aisl_stream_close_headers(stream) != AISL_SUCCESS)
			return -1;
	}

	if (d_len == -1)
		d_len = strlen(data);

	return buffer_append(&stream->buffer, data, d_len);
}


__attribute__ ((visibility ("default") ))
int
aisl_puts(const char *str, AislStream stream)
{
	if (stream->state < AISL_STREAM_STATE_SEND_BODY) {
		if (aisl_stream_close_headers(stream) != AISL_SUCCESS)
			return -1;
	}
	return aisl_write( stream, str, -1);
}


__attribute__ ((visibility ("default") ))
AislInstance
aisl_get_instance(AislStream stream)
{
	return stream->client->server->instance;
}


__attribute__ ((visibility ("default") ))
void
aisl_set_output_event(AislStream stream, bool value)
{
	if (value)
		stream->flags |= FLAG_OUTPUT_CHUNKED;
	else if (stream->flags & FLAG_OUTPUT_CHUNKED)
		stream->flags &= ~FLAG_OUTPUT_CHUNKED;
}


__attribute__ ((visibility ("default") ))
bool
aisl_get_output_event(AislStream stream)
{
	return (stream->flags & FLAG_OUTPUT_CHUNKED);
}

