#ifndef _AISL_STREAM_H_
#define _AISL_STREAM_H_

#include <arpa/inet.h>
/* Library statuses */
/* HTTP requests */
#include <aisl/http.h>
#include <aisl/status.h>

/* -------------------------------------------------------------------------- */

typedef struct sockaddr_in * aisl_server_t;
typedef struct sockaddr_in * aisl_client_t;

struct aisl_stream
{
  /* DO NOT USE PROPERTIES DIRECTLY IN NEW CODE */

  struct sockaddr_in    *client;

  const char            *host;
  const char            *path;
  const char            *query;
  const char            *scheme;

  void                  *u_ptr;     /* pointer to bind custom data to stream */

  aisl_http_method_t     request_method;
};

/* pointer to stream descriptor */
typedef struct aisl_stream * aisl_stream_t;

/* -------------------------------------------------------------------------- */

/* start response to client
 * this function should be called before any header or content function
 * necessary protocol headers as well as Content-Type and Content-Length
 * will be set automaticaly
 * @stream         : stream instance
 * @status         : HTTP response status code (default 200)
 * @content_type   : string with content type (default text/html), NULL -> no
 * @content_length : length of content, 0 = no content
 * */
aisl_status_t
aisl_response(aisl_stream_t stream, aisl_http_response_t  status_code,
                                    const char           *content_type,
                                    uint32_t              content_length);

/* send all buffered data to client
 * ALL responses should always be finished with calling of this method
 * @stream         : stream instance
 * */
aisl_status_t
aisl_flush(aisl_stream_t stream);

/* header functions --------------------------------------------------------- */

/* add custom header to stream
 * this function should be called before content functions
 * @stream : stream instance
 * @key    : header key string
 * @value  : header value string
 * */
int
aisl_header(aisl_stream_t stream, const char *key, const char *value);

/* add custom header to stream
 * this function should be called before content functions
 * @stream  : stream instance
 * @key     : header key string
 * @f_value : value format string, same as for printf function
 * @...     : arguments according to f_value string
 * */
int
aisl_header_printf(aisl_stream_t stream, const char *key,
                                         const char *f_value, ...);

/* add custom header to stream
 * this function should be called before content functions
 * @stream  : stream instance
 * @key     : header key string
 * @f_value : value format string, same as for printf function
 * @args    : arguments macro according to f_value string
 * */
int
aisl_header_vprintf(aisl_stream_t stream, const char *key,
                                          const char *format,
                                          va_list     args);


/* data response functions -------------------------------------------------- */

/* response formated data to client
 * @stream : stream instance
 * @format : format string, same as for printf
 * @...    : arguments according to format string
 * @result : number of responed bytes
 * */
int
aisl_printf(aisl_stream_t stream, const char *format, ...);


/* response formated data to client
 * @stream : stream instance
 * @format : format string, same as for printf
 * @args   : arguments macro according to format string
 * @result : number of responed bytes
 * */
int
aisl_vprintf(aisl_stream_t stream, const char *format, va_list args);

/* response characters to client
 * @stream : stream instance
 * @data   : characters to be sent
 * @d_len  : number of characters to send
 * @result : number of responed bytes
 * */
int
aisl_write(aisl_stream_t s, const char *data, int d_len);

/* response string to client
 * @string : string to be sent
 * @stream : stream instance
 * @result : number of responed bytes
 * */
int
aisl_puts(const char *string, aisl_stream_t stream);

/* -------------------------------------------------------------------------- */

void
aisl_cancel(aisl_stream_t s);

/* -------------------------------------------------------------------------- */

bool
aisl_is_secure(aisl_stream_t s);

/* -------------------------------------------------------------------------- */

void *
aisl_get_context(aisl_stream_t s);

/* -------------------------------------------------------------------------- */

void
aisl_set_context(aisl_stream_t s, void * u_ptr);

/* -------------------------------------------------------------------------- */

aisl_client_t
aisl_get_client(aisl_stream_t s);

/* -------------------------------------------------------------------------- */

aisl_server_t
aisl_get_server(aisl_stream_t s);

  /* -------------------------------------------------------------------------- */

aisl_http_version_t
aisl_get_http_version(aisl_stream_t s);

/* -------------------------------------------------------------------------- */

void
aisl_reject( aisl_stream_t s);

/* -------------------------------------------------------------------------- */

#endif
