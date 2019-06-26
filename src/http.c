/******************************************************************************
 *
 *                Copyright (c) 2017-2019 by Löwenware Ltd
 *             Please, refer LICENSE file for legal information
 *
 ******************************************************************************/

/**
 * @file http.c
 * @author Ilja Kartašov <ik@lowenware.com>
 * @brief HTTP module source file
 *
 * @see https://lowenware.com/aisl/
 */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "stream.h"
#include "debug.h"
#include "http.h"


static AislHttpMethod
http_method_from_string(const char *method, int32_t length)
{
	int i;
	AislHttpMethod methods[3] = {0, 0, 0};

	switch(length) {
	case 3:
		methods[0] = AISL_HTTP_GET;
		methods[1] = AISL_HTTP_PUT;
		methods[2] = AISL_HTTP_PRI;
		break;

	case 4:
		methods[0] = AISL_HTTP_POST;
		methods[1] = AISL_HTTP_HEAD;
		break;

	case 5:
		methods[0] = AISL_HTTP_TRACE;
		break;

	case 6:
		methods[0] = AISL_HTTP_DELETE;
		break;

	case 7:
		methods[0] = AISL_HTTP_OPTIONS;
		methods[1] = AISL_HTTP_CONNECT;
		break;
	}

	for (i=0; i<sizeof (methods)/sizeof (AislHttpMethod); i++) {
		if (!(methods[i]))
			break;

		if (strcmp(method, aisl_http_method_to_string(methods[i]))==0)
			return methods[i];
	}

	return AISL_HTTP_METHOD_UNKNOWN;
}


static AislHttpVersion
http_version_from_string(const char *version_string)
{
	if (strncmp(version_string, "HTTP/", 5)==0) {
		if (strncmp(&version_string[5], "0.9", 3)==0) return AISL_HTTP_0_9;
		if (strncmp(&version_string[5], "1.0", 3)==0) return AISL_HTTP_1_0;
		if (strncmp(&version_string[5], "1.1", 3)==0) return AISL_HTTP_1_1;
		if (strncmp(&version_string[5], "2.0", 3)==0) return AISL_HTTP_2_0;
	}

	return 0;
}

/* Library Level */

ParserStatus
http_10_parse_request(char *data, int32_t *p_size, AislStream stream)
{
	/* STEP 1. Split data according to HTTP request format
	 *
	 * GET http://lowenware.com:80/index.html?param=value HTTP/1.1\r\n
	 * ^   ^      ^             ^ ^           ^           ^         ^
	 * |   |      |             | |           |           |         |
	 * |   |      |             | |           |           |         +--- newline
	 * |   |      |             | |           |           +------------- version
	 * |   |      |             | |           +------------------------- query
	 * |   |      |             | +------------------------------------- path
	 * |   |      |             +--------------------------------------- port
	 * |   |      +----------------------------------------------------- host
	 * |   +------------------------------------------------------------ uri
	 * +---------------------------------------------------------------- method
	 */
	char *uri        = NULL,
			 *uri_end    = NULL,
			 *host       = NULL,
			 *port       = NULL,
			 *path       = NULL,
			 *query      = NULL,
			 *version    = NULL,
			 *newline    = NULL,
			 *method     = data,
			 *method_end = NULL;

	AislHttpMethod  http_method;
	AislHttpVersion http_version;

	int32_t size = *p_size;

	while(!newline && size--) {
		switch(*data)
		{
		case ' ':
			if (!method_end)
				method_end = data;
			else if (path && !uri_end)
				uri_end = data;
			break;

		case ':':
			if (uri && !host) {
				host = data+3;
			} else if (host && !port) {
				port = data+1;
			} else if (version) {
				DPRINTF("bad character in HTTP version (%c)", *data);
				return HTTP_PARSER_ERROR;
			}
			break;

		case '/':
			if (!path && data > host) {
				path = data;
				if (!uri)
					uri = path;
			} else if (version && data-version != 4) {
				DPRINTF("wrong HTTP version length");
				return HTTP_PARSER_ERROR;
			}
			break;

		case '?':
			if (!query) {
				query = data+1;
			} else if (version) {
				DPRINTF("bad character in HTTP version (%c)", *data);
				return HTTP_PARSER_ERROR;
			}
			break;

		case '\n':
			newline = data;
			break;

		case '\r':
			if (!version) {
				DPRINTF("unexpected end of HTTP request");
				return HTTP_PARSER_ERROR;
			}
			break;

		default:
			if (!uri && method_end) {
				uri = data;
			} else if (!version && uri_end) {
				version = data;
			} else if (version && data-version > 7) {
				DPRINTF("bad HTTP version length");
				return HTTP_PARSER_ERROR;
			}
		}
		data++;
	}

	/* STEP 2. Verify splitting was completed */

	/* Was request sent? */
	if (!newline)
		return HTTP_PARSER_HUNGRY;

	/* Check mandatory parts presence */
	if (!method_end || !path || !uri_end || !version) {
		DPRINTF("parser error: method=%d, path=%d, uri_end=%d, version=%d",
				(method_end ? 1 : 0), (path ? 1 : 0), (uri_end ? 1 : 0),
				(version ? 1: 0));
		return HTTP_PARSER_ERROR;
	}

	*method_end = 0;
	*newline    = 0;
	*uri_end    = 0;

	http_method = http_method_from_string(method, method_end - method);
	if (http_method == AISL_HTTP_METHOD_UNKNOWN) {
		DPRINTF("invalid HTTP method");
		return HTTP_PARSER_ERROR;
	}

	if ((http_version = http_version_from_string(version))==0) {
		DPRINTF("invalid HTTP version");
		return HTTP_PARSER_ERROR;
	}

	if (query) {
		*(query - 1) = 0;
	} else {
		query = uri_end;
	}

	if (host) {
		if (strncmp(uri, "http://", 7) || strncmp(uri, "https://", 8)) {
			DPRINTF("invalid HTTP uri");
			return HTTP_PARSER_ERROR;
		}
	
		if (port)
			*(port - 1) = 0;
	}

	stream->client->http_version = http_version;
	aisl_stream_set_request(stream, http_method, path, query);

	if (host)
		aisl_stream_set_header(stream, "host", host);
	/* how many characters has been read */
	*(p_size) = size;
	return HTTP_PARSER_SUCCESS;
}


ParserStatus
http_10_parse_header(char *data, int32_t *p_size, AislStream stream)
{
	int32_t size = *p_size;
	char *key     = data,
			 *colon   = NULL,
			 *val     = NULL,
			 *val_end = NULL,
			 *newline = NULL;


	DPRINTF("parse header: %p, %d, %02X %02X", data, *p_size, *data & 0xFF, *(data+1) & 0xFF);

	while(!newline && size-- ) {
		switch(*data) {
		case ' ':
			if (val && !val_end)
				val_end = data;
			break;

		case ':':
			if (!colon) {
				if (colon == key) {
					DPRINTF("parser error: nameless HTTP header");
					return HTTP_PARSER_ERROR;
				}

				colon = data;
			}
			break;

		case '\n':
			newline = data;

		case '\r':
			if (!val_end && val)
				val_end = data;
			break;

		default:
			if (!colon) {
				*data = tolower(*data);
			} else if (!val) {
				if (colon)
					val = data;
			}

			if (val_end)
				val_end = NULL;
		}
		data++;
	}

	if (!newline)
		return HTTP_PARSER_HUNGRY;

	/* DPRINTF("(%p == %p); *key == 0x%02x", newline, key, *key & 0xFF); */

	if (colon && val && val_end) {
		*colon   = 0;
		*val_end = 0;
		aisl_stream_set_header(stream, key, val);
		*p_size = size;
		return HTTP_PARSER_SUCCESS;
	} else if (newline == key || (newline == key+1 && *key == '\r')) {
		*p_size = size;
		DPRINTF("end of headers received");
		return (aisl_stream_set_end_of_headers(stream) == 0) ?
		  HTTP_PARSER_READY : HTTP_PARSER_SUCCESS;
	}
	DPRINTF("parser error: invalid HTTP header");
	return HTTP_PARSER_ERROR;
}


ParserStatus
http_10_parse_body(char *data, int32_t *p_size, AislStream stream)
{
	int32_t size = *p_size;

	if (!size)
		return HTTP_PARSER_HUNGRY;

	*p_size = 0;

	switch (aisl_stream_set_body(stream, data, size)) {
	case  0:
		return HTTP_PARSER_READY;
	case -1:
		DPRINTF("parser error: invalid HTTP body length");
		return HTTP_PARSER_ERROR;
	default:
		return HTTP_PARSER_SUCCESS;
	}
}


/* API Level */

__attribute__ ((visibility ("default") ))
const char *
aisl_http_version_to_string(AislHttpVersion version)
{
	switch (version) {
	case AISL_HTTP_0_9:
		return "HTTP/0.9";
	case AISL_HTTP_1_0:
		return "HTTP/1.0";
	case AISL_HTTP_1_1:
		return "HTTP/1.1";
	case AISL_HTTP_2_0:
		return "HTTP/2.0";
	}
	return "";
}


__attribute__ ((visibility ("default") ))
const char *
aisl_http_response_to_string(AislHttpResponse code)
{
	switch (code) {
	/* most common for faster behavior */
	case AISL_HTTP_OK: return "OK";
	case AISL_HTTP_MOVED_PERMANENTLY: return "Moved Permanently";
	/* informational */
	case AISL_HTTP_CONTINUE: return "Continue";
	case AISL_HTTP_SWITCHING_PROTOCOLS: return "Switching Protocols";
	/* Successful */
	case AISL_HTTP_CREATED: return "Created";
	case AISL_HTTP_ACCEPTED: return "Accepted";
	case AISL_HTTP_NON_AUTHORITATIVE_INFORMATION: return "Non-Authoritative Information";
	case AISL_HTTP_NO_CONTENT: return "No Content";
	case AISL_HTTP_RESET_CONTENT: return "Reset Content";
	case AISL_HTTP_PARTIAL_CONTENT: return "Partial Content";
	/* redirection */
	case AISL_HTTP_MULTIPLE_CHOICES: return "Multiple Choices";
	case AISL_HTTP_FOUND: return "Found";
	case AISL_HTTP_SEE_OTHER: return "See other";
	case AISL_HTTP_NOT_MODIFIED: return "Not Modified";
	case AISL_HTTP_USE_PROXY: return "Use Proxy";
	case AISL_HTTP_UNUSED: return "(unused)";
	case AISL_HTTP_TEMPORARY_REDIRECT: return "Temporary Redirect";
	/* client error */
	case AISL_HTTP_BAD_REQUEST: return "Bad Request";
	case AISL_HTTP_UNAUTHORIZED: return "Unauthorized";
	case AISL_HTTP_PAYMENT_REQUIRED: return "Payment Required";
	case AISL_HTTP_FORBIDDEN: return "Forbidden";
	case AISL_HTTP_NOT_FOUND: return "Not Found";
	case AISL_HTTP_METHOD_NOT_ALLOWED: return "Method Not Allowed";
	case AISL_HTTP_NOT_ACCEPTABLE: return "Not Acceptable";
	case AISL_HTTP_PROXY_AUTHENTICATION_REQUIRED: return "Proxy Authentication Required";
	case AISL_HTTP_REQUEST_TIMEOUT: return "Request Timeout";
	case AISL_HTTP_CONFLICT: return "Conflict";
	case AISL_HTTP_GONE: return "Gone";
	case AISL_HTTP_LENGTH_REQUIRED: return "Length Required";
	case AISL_HTTP_PRECONDITION_FAILED: return "Precondition Failed";
	case AISL_HTTP_REQUEST_ENTITY_TOO_LARGE: return "Request Entity Too Large";
	case AISL_HTTP_REQUEST_URI_TOO_LONG: return "Request-URI Too Long";
	case AISL_HTTP_UNSUPPORTED_MEDIA_TYPE: return "Unsupported Media Type";
	case AISL_HTTP_REQUESTED_RANGE_NOT_SATISFIABLE: return "Requested Range Not Satisfiable";
	case AISL_HTTP_EXPECTATION_FAILED: return "Expectation Failed";
	/* server error */
	case AISL_HTTP_INTERNAL_SERVER_ERROR: return "Internal Server Error";
	case AISL_HTTP_NOT_IMPLEMENTED: return "Not Implemented";
	case AISL_HTTP_BAD_GATEWAY: return "Bad Gateway";
	case AISL_HTTP_SERVICE_UNAVAILABLE: return "Service Unavailable";
	case AISL_HTTP_GATEWAY_TIMEOUT: return "Gateway Timeout";
	case AISL_HTTP_VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";
	}
	return "";
}


__attribute__ ((visibility ("default") ))
const char *
aisl_http_method_to_string( AislHttpMethod method )
{
	switch(method) {
	case AISL_HTTP_GET:     return "GET";
	case AISL_HTTP_PUT:     return "PUT";
	case AISL_HTTP_POST:    return "POST";
	case AISL_HTTP_HEAD:    return "HEAD";
	case AISL_HTTP_TRACE:   return "TRACE";
	case AISL_HTTP_DELETE:  return "DELETE";
	case AISL_HTTP_OPTIONS: return "OPTIONS";
	case AISL_HTTP_CONNECT: return "CONNECT";
	case AISL_HTTP_PRI:     return "PRI";
	case AISL_HTTP_METHOD_UNKNOWN: break;
	}
	return "";
}
