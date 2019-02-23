#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <aisl/status.h>

#include "stream.h"
#include "globals.h"
#include "client.h"
#include "handle.h"

static void
pair_free( pair_t self )
{
  if (!self) return;

  if(self->key) free(self->key);
  if(self->value) free(self->value);
  free(self);
}
/* -------------------------------------------------------------------------- */

stream_t
stream_new(struct sockaddr_in  *client, int id, stream_state_t state)
{
  stream_t self = malloc(sizeof(struct stream));

  if (self)
  {
    /* public data */
    ASTREAM(self)->client         = client;
    ASTREAM(self)->host           = NULL;
    ASTREAM(self)->path           = NULL;
    ASTREAM(self)->query          = NULL;
    ASTREAM(self)->scheme         = NULL;
    ASTREAM(self)->u_ptr          = NULL;
    ASTREAM(self)->request_method = AISL_HTTP_GET;

    /* private data */
    self->headers                 = NULL; /* request headers */
    self->buffer                  = buffer_new(0);

    self->c_type                  = NULL;

    self->response                = AISL_HTTP_OK;
    self->state                   = STREAM_REQUEST_METHOD;
    self->c_length                = 0;
    self->c_offset                = 0; /* headers length */
    self->id                      = id;
    self->c_length_unknown        = true;
    self->flags                   = 0;
  }
  return self;
}

/*
stream_t
stream_reset(stream_t self)
{
  if (ASTREAM(self)->path)
  {
    free( (char*) ASTREAM(self)->path);
    ASTREAM(self)->path = NULL;
  }

  if (ASTREAM(self)->query)
  {
    free( (char*) ASTREAM(self)->query);
    ASTREAM(self)->query = NULL;
  }

  ASTREAM(self)->u_ptr          = NULL;
  ASTREAM(self)->request_method = AISL_HTTP_GET;

  if (self->headers)
  {
    list_free(self->headers, (list_destructor_t) pair_free);
    self->headers = NULL;
  }

  self->c_type                  = NULL;
  self->response                = AISL_HTTP_OK;
  self->state                   = STREAM_REQUEST_METHOD;
  self->c_length                = 0;
  self->c_offset                = 0; / * headers length * /
  self->c_length_unknown        = true;
  self->flags                   = 0;

  return self;
}
*/
/* -------------------------------------------------------------------------- */

void
stream_free(stream_t self)
{
  if (self->buffer)  buffer_free(self->buffer);
  if (self->headers) list_free(self->headers, (list_destructor_t) pair_free);

  if (ASTREAM(self)->path)  free( (char*) ASTREAM(self)->path);
  if (ASTREAM(self)->query) free( (char*) ASTREAM(self)->query);

  aisl_handle_t hd =  ((client_t) ASTREAM(self)->client)->server->owner;
  aisl_raise_event(hd, self, AISL_STREAM_CLOSE);
  ASTREAM(self)->u_ptr = NULL;
  aisl_remove_listeners_for(hd, self);
  free(self);
}

/* -------------------------------------------------------------------------- */

int
stream_write(stream_t self, const char * data, uint32_t d_len)
{
  return buffer_add( self->buffer, data, d_len);
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
void
aisl_cancel(aisl_stream_t s)
{
  client_close( (client_t) s->client );
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
bool
aisl_is_secure(aisl_stream_t s)
{
  client_t cli = (client_t) s->client;

  return (cli->ssl) ? true : false;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
void *
aisl_get_context(aisl_stream_t s)
{
  return s->u_ptr;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
void
aisl_set_context(aisl_stream_t s, void * u_ptr)
{
  s->u_ptr = u_ptr;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_client_t
aisl_get_client(aisl_stream_t s)
{
  return s->client;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_server_t
aisl_get_server(aisl_stream_t s)
{
  return  (aisl_server_t) (((client_t) s->client)->server);
}


/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_http_version_t
aisl_get_http_version(aisl_stream_t s)
{
  client_t cli = (client_t) s->client;

  return cli->protocol;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
void
aisl_reject(aisl_stream_t s)
{
  client_t cli = (client_t) s->client;

  client_close( cli );
}

/* -------------------------------------------------------------------------- */
