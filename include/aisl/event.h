#ifndef _AISL_EVENT_H_
#define _AISL_EVENT_H_

#include <stdbool.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <aisl/stream.h>
#include <aisl/status.h>
#include <aisl/http.h>

/* -------------------------------------------------------------------------- */

typedef unsigned int aisl_event_t;
/* -------------------------------------------------------------------------- */

typedef enum
{
  AISL_SERVER_OPEN        = 100,
  AISL_SERVER_ERROR       = 190,

  AISL_CLIENT_CONNECT     = 200,
  AISL_CLIENT_DISCONNECT  = 210,
  AISL_CLIENT_TIMEOUT     = 220,

  AISL_STREAM_OPEN        = 300,   /* 5 - headers recieved */
  AISL_STREAM_HEADER      = 310, /* 5 - headers recieved */
  AISL_STREAM_INPUT       = 320, /* 6 - chunk of data transmission */
  AISL_STREAM_REQUEST     = 330,   /* 7 - data received, response required */
  AISL_STREAM_OUTPUT      = 340,/* event for long-size responses optimal handling */
  AISL_STREAM_CLOSE       = 350,
  AISL_STREAM_ERROR       = 390,   /* 8 - bad request */

  AISL_EVENTS_CUSTOM      = 999

} aisl_event_id_t;


/* -------------------------------------------------------------------------- */

/* AISL_SERVER_OPEN event handler */
typedef bool
(*aisl_server_open_t)(       aisl_server_t        server,
                             int                  flags );

/* AISL_SERVER_ERROR event handler */
typedef bool
(*aisl_server_error_t)(      aisl_server_t        server,
                             int                  flags,
                             const char         * details );

/* AISL_CLIENT_CONNECT event handler */
typedef bool
(*aisl_client_connect_t)(    aisl_server_t        server,
                             aisl_client_t        client );

/* AISL_CLIENT_DISCONNECT event handler */
typedef bool
(*aisl_client_disconnect_t)( aisl_server_t        server,
                             aisl_client_t        client );

/* AISL_CLIENT_DISCONNECT event handler */
typedef bool
(*aisl_client_timeout_t)(    aisl_server_t        server,
                             aisl_client_t        client );

/* AISL_STREAM_OPEN event handler */
typedef bool
(*aisl_stream_open_t)(       aisl_stream_t        s,
                             aisl_http_method_t   method,
                             const char         * path,
                             const char         * query );

typedef bool
(*aisl_stream_header_t)(     aisl_stream_t        s,
                             const char         * key,
                             const char         * value );


/* AISL_STREAM_INPUT event handler */
typedef bool
(*aisl_stream_input_t)(      aisl_stream_t        s,
                             char               * data,
                             int                  len );

/* AISL_STREAM_REQUEST event handler */
typedef bool
(*aisl_stream_request_t)(    aisl_stream_t        s );

/* AISL_STREAM_OUTPUT event handler */
typedef bool
(*aisl_stream_output_t)(     aisl_stream_t        s,
                             uint32_t             buffer_space );

typedef bool
(*aisl_stream_close_t)(      aisl_stream_t        s );

/* AISL_STREAM_ERROR event handler */
typedef bool
(*aisl_stream_error_t)(      aisl_stream_t        s,
                             const char         * details );

/* CUSTOM event_handler */
typedef bool
(*aisl_custom_event_t)(      void               * source,
                             va_list              vl );

/* on delay timeout */
typedef bool
(*aisl_delay_timeout_t)(     void               * u_data );

/* -------------------------------------------------------------------------- */

/* type for event callbacks to use in structures and function prototypes */
typedef bool
(* aisl_callback_t) (void);

/* cast callback as aisl_callback_t */
#define AISL_CALLBACK(x) ((aisl_callback_t) x)

/* -------------------------------------------------------------------------- */

const char *
aisl_event_get_text( aisl_event_t e_id );


/* -------------------------------------------------------------------------- */

#endif
