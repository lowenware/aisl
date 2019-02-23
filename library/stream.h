#ifndef _AISL_STREAM_H__
#define _AISL_STREAM_H__

#include <stdbool.h>
#include <aisl/stream.h>
#include <cStuff/list.h>
#include "buffer.h"

/* -------------------------------------------------------------------------- */

#define STREAM_FLAG_OUTPUT_READY   (1<<0)
#define STREAM_FLAG_OUTPUT_CHUNKED (1<<1)

/* -------------------------------------------------------------------------- */

struct pair
{
  char *key;
  char *value;
};

typedef struct pair * pair_t;

/* -------------------------------------------------------------------------- */

typedef enum {

  STREAM_REQUEST_METHOD,   
  STREAM_REQUEST_PATH,
  STREAM_REQUEST_PROTOCOL, 

  STREAM_REQUEST_HEADER_KEY,    /* HTTP1 header key */
  STREAM_REQUEST_HEADER_VALUE,  /* HTTP1 header value */

  STREAM_REQUEST_CONTENT,          /* HTTP1 data value */

  /* states below show stream state 
   * and do not show what data was sent to client
   * */
  STREAM_REQUEST_READY,

  STREAM_RESPONSE_HEADER,
  STREAM_RESPONSE_CONTENT,
  STREAM_RESPONSE_READY

} stream_state_t;


/* real wrapper for aisl_stream_t */
struct stream
{
  struct aisl_stream     _public;

  /* private data */
  list_t                 headers; /* request headers */
  buffer_t               buffer;

  const char            *c_type;

  aisl_http_response_t   response;
  stream_state_t         state;
  uint32_t               c_length;
  uint32_t               c_offset;
  int                    id;
  int                    flags;

  bool                   c_length_unknown;

};

typedef struct stream * stream_t;

#define STREAM(x) ((stream_t) x)
#define ASTREAM(x) ((aisl_stream_t) x)

/* -------------------------------------------------------------------------- */

stream_t
stream_new(struct sockaddr_in  *client, int id, stream_state_t state);

/* -------------------------------------------------------------------------- */

void
stream_free(stream_t self);

/* -------------------------------------------------------------------------- */

stream_t
stream_reset(stream_t self);

/* -------------------------------------------------------------------------- */

#endif
