#include <stdlib.h>
#include <string.h>

#include <openssl/err.h>

#include <aisl/handle.h>
#include <cStuff/str-utils.h>
#include <cStuff/list.h>
#include "stream.h"
#include "server.h"
#include "client.h"
#include "globals.h"
#include "handle.h"

/* -------------------------------------------------------------------------- */

struct listener
{
  void            *source;
  aisl_callback_t  cb;
  aisl_event_t     e_id;
};

typedef struct listener * listener_t;

/* -------------------------------------------------------------------------- */

struct delay
{
  struct timespec  next;
  uint32_t         delay;
  void            *u_data;
  aisl_callback_t  cb;
};

typedef struct delay * delay_t;

/* -------------------------------------------------------------------------- */

struct crypter
{
  char    * keyFile;
  char    * crtFile;
  char    * srvName;
  SSL_CTX * sslCtx;
};

typedef struct crypter * crypter_t;



/* -------------------------------------------------------------------------- */
static int gHandles = 0;

/* -------------------------------------------------------------------------- */

static listener_t
listener_new( void * source, aisl_event_t e_id, aisl_callback_t cb)
{
  listener_t self = malloc(sizeof(struct listener));
  if (self)
  {
    self->source = source;
    self->e_id   = e_id;
    self->cb     = cb;
  }

  return self;
}

/* -------------------------------------------------------------------------- */

void
aisl_remove_listeners_for( aisl_handle_t self, void * source )
{
  int i=self->listeners->count-1;
  while ( !(i < 0) )
  {
    listener_t listener = list_index(self->listeners, i);
    if ( listener->source == source )
    {
      free(listener);
      list_remove_index(self->listeners, i);
    }

    i--;
  }
}

/* -------------------------------------------------------------------------- */

static void
crypter_free( crypter_t self )
{
  if (self->srvName)
    free(self->srvName);

  if (self->keyFile)
  {
    free(self->keyFile);
    SSL_CTX_free(self->sslCtx);
  }

  if (self->crtFile)
    free(self->crtFile);

  free(self);
}

/* -------------------------------------------------------------------------- */

static crypter_t
crypter_new( const char * server_name,
             const char * key_file,
             const char * crt_file )
{
  crypter_t self;

  if ( (self=calloc(1, sizeof(struct crypter))) != NULL )
  {
    if (!(self->srvName = str_copy( server_name ? server_name : "*" )))
      goto release;

    if ( key_file && !(self->keyFile = str_copy(key_file)))
      goto release;

    if ( crt_file && !(self->crtFile = str_copy(crt_file)))
      goto release;

  }

  goto finally;


release:
  crypter_free(self);
  self = NULL;

finally:
  return self;
}

/* -------------------------------------------------------------------------- */

static bool
delay_is_expired(delay_t self)
{
  if (!self->delay) return true;

  struct timespec tv;
  clock_gettime(CLOCK_REALTIME, &tv);

  /*
  printf("> %ld.%ld & %ld.%ld\n", self->next.tv_sec, self->next.tv_nsec,
                                  tv.tv_sec, tv.tv_nsec);
                                  */

  if (tv.tv_sec > self->next.tv_sec)
    return true;
  else if (tv.tv_sec == self->next.tv_sec && tv.tv_nsec >= self->next.tv_nsec)
    return true;

  return false;
}

/* -------------------------------------------------------------------------- */

static void
delay_reset(delay_t self)
{
  clock_gettime(CLOCK_REALTIME, &self->next);

  self->next.tv_sec  += self->delay / 1000;
  self->next.tv_nsec += (self->delay % 1000) * 1000000;

  self->next.tv_sec += self->next.tv_nsec / 1000000000;
  self->next.tv_nsec = self->next.tv_nsec % 1000000000;

}

/* -------------------------------------------------------------------------- */

static delay_t
delay_new(aisl_callback_t cb, uint32_t delay, void  *u_data)
{
  delay_t self = malloc(sizeof(struct delay));

  if (self)
  {
    self->cb     = cb;
    self->u_data = u_data;
    self->delay  = delay;

    delay_reset(self);
  }

  return self;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_handle_t
aisl_handle_new( size_t min_clients, size_t buffer_size )
{
  aisl_handle_t self;

  if ((gHandles++) == 0)
  {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
  }

  if ( !(self = calloc(1, sizeof(struct aisl_handle))) )
    goto finally;

  if ( !(self->servers = list_new(1)) )
    goto release;

  if ( !(self->clients = list_new(min_clients)) )
    goto release;

  if ( !(self->listeners = list_new(16)) )
    goto release;

  if ( !(self->buffer = buffer_new(buffer_size)) )
    goto release;

  if ( !(self->crypters = list_new(0)) )
    goto release;

  if ( !(self->delays = list_new(0)) )
    goto release;

  goto finally;

release:
  aisl_handle_free(self);
  self = NULL;

finally:
  return self;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_handle_t
aisl_handle_from_stream( aisl_stream_t s )
{
  return ((client_t)s->client)->server->owner;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
void
aisl_handle_free( aisl_handle_t self )
{
  if ((--gHandles) == 0)
  {
    EVP_cleanup();
  }

  if (self->clients)
    list_free(self->clients, (list_destructor_t) client_free );

  if (self->servers)
    list_free(self->servers, (list_destructor_t)  server_free );

  if (self->listeners)
    list_free(self->listeners, free);
  
  if (self->delays)
    list_free(self->delays, free);

  if (self->buffer)
    buffer_free(self->buffer);

  if (self->crypters)
    list_free(self->crypters, (list_destructor_t)  crypter_free );

  if (self->lastError)
    free(self->lastError);

  free(self);
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
void
aisl_handle_set_error( aisl_handle_t self, const char * err_msg )
{
  if (self->lastError)
    free(self->lastError);

  self->lastError = str_copy(err_msg);
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_status_t
aisl_bind( aisl_handle_t self, const char * address, int port, int flags )
{
  server_t      server;

  if ( !(server = server_new(address, port)) )
    goto finally;

  server->owner = self;
  server->flags |= (flags & AISL_FLAG_SSL);

  if ( list_append(self->servers, server) == -1 )
    goto release;

  goto finally;

release:
  server_free(server);
  server = NULL;

finally:
  return server ? AISL_SUCCESS : AISL_MALLOC_ERROR;
}

/* -------------------------------------------------------------------------- */

static int
get_ssl_context( SSL * ssl, int * ptr, void * handle )
{
  const char * server_name = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);

  SSL_CTX * ctx = aisl_get_ssl_ctx( (aisl_handle_t) handle, server_name );

  if (ctx)
  {
    SSL_set_SSL_CTX(ssl, ctx);
  }

  return SSL_TLSEXT_ERR_OK;
}

/* -------------------------------------------------------------------------- */

static SSL_CTX *
create_ssl_context( aisl_handle_t self, 
                    const char * key_file, 
                    const char * crt_file )
{
  const SSL_METHOD * method;
  SSL_CTX * ctx;

  method = SSLv23_server_method();

  if ( !(ctx = SSL_CTX_new(method)) )
    goto except;

  SSL_CTX_set_ecdh_auto(ctx, 1);

  SSL_CTX_set_tlsext_servername_callback( ctx, get_ssl_context );
  SSL_CTX_set_tlsext_servername_arg(      ctx, (void *) self );

  if (!(SSL_CTX_use_certificate_file(ctx, crt_file, SSL_FILETYPE_PEM) > 0))
    goto release;

  if (!(SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) > 0))
    goto release;

  goto finally;

release:
  SSL_CTX_free(ctx);
  ctx = NULL;

except:
  aisl_handle_set_error( self, ERR_error_string(ERR_get_error(),NULL) );

finally:
  return ctx;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_status_t
aisl_set_ssl( aisl_handle_t self, const char * server_name,
                                  const char * key_file,
                                  const char * crt_file )
{
  SSL_CTX * ssl_ctx = NULL;
  int i;
  crypter_t crypter;

  /* lookup for existing contexts */
  for (i=0; i<self->crypters->count; i++)
  {
    crypter = list_index(self->crypters, i);
    if (crypter->keyFile && strcmp(crypter->keyFile, key_file)==0 &&
        crypter->crtFile && strcmp(crypter->crtFile, crt_file)==0 )
    {
      ssl_ctx = crypter->sslCtx;
      key_file = NULL;
      crt_file = NULL;
      break;
    }
  }

  if (! (crypter = crypter_new(server_name, key_file, crt_file)) )
  {
    return AISL_MALLOC_ERROR;
  }

  if (! ssl_ctx)
  {
    if (!(ssl_ctx = create_ssl_context(self, key_file, crt_file)))
    {
      crypter_free(crypter);
      return AISL_EXTCALL_ERROR;
    }
  }

  crypter->sslCtx = ssl_ctx;

  if (list_append(self->crypters, crypter)==-1)
  {
    crypter_free(crypter);
    return AISL_MALLOC_ERROR;
  }

  return AISL_SUCCESS;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
SSL_CTX *
aisl_get_ssl_ctx( aisl_handle_t self, const char * server_name )
{
  int i;
  crypter_t crypter;

  for (i=0; i<self->crypters->count; i++)
  {
    crypter = list_index(self->crypters, i);
    if (server_name)
    {
      if (strcmp(crypter->srvName, server_name)!=0)
        continue;
    }

    return crypter->sslCtx;
  }

  return NULL;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_status_t
aisl_set_callback( aisl_handle_t    self,
                   void           * source,
                   aisl_event_t     e_id,
                   aisl_callback_t  cb  )
{
  listener_t listener;

  if (! (listener = listener_new(source, e_id, cb)) )
    return AISL_MALLOC_ERROR;

  if (list_append(self->listeners, listener) == -1)
  {
    free(listener);
    return AISL_MALLOC_ERROR;
  }

  if (e_id == AISL_STREAM_OUTPUT) /* subscribtion for chunked output */
  {
    if (source)
    {
      ( (stream_t) source )->flags |= STREAM_FLAG_OUTPUT_CHUNKED;
    }
  }
  else if (e_id == AISL_STREAM_OPEN)
  {
    self->flags |= AISL_HANDLE_HAS_STREAM_LISTENERS;
  }

  return AISL_SUCCESS;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_status_t
aisl_set_delay( aisl_handle_t     self,
                aisl_callback_t   cb,
                uint32_t          usec,
                void            * u_data )
{
  delay_t delay = delay_new(cb, usec, u_data);
  if (!delay)
    return AISL_MALLOC_ERROR;

  if (list_append(self->delays, delay) == -1)
  {
    free(delay);
    return AISL_MALLOC_ERROR;
  }

  return AISL_SUCCESS;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
bool
aisl_raise_event( aisl_handle_t   self,
                  void          * source,
                  aisl_event_t    e_id,
                                  ... )
{
  va_list vl;
  bool result;

  va_start(vl, e_id);
  result = aisl_raise_event_vl(self, source, e_id, vl);
  va_end(vl);

  return result;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
bool
aisl_raise_event_vl( aisl_handle_t   self,
                     void          * source,
                     aisl_event_t    e_id,
                     va_list         vl )
{
  int    i,
         i_val;
  listener_t lst;
  bool res = false;

  char * c_ptr,
       * c_ptr2;

  for(i=self->listeners->count-1; i>=0; i--)
  {
    lst = list_index(self->listeners, i);

    /*printf("AISL> raise %s:%s, %p:%p\n", _event_text(e_id), _event_text(lst->e_id), source, lst->source);*/
    if (lst->e_id == e_id && (source == lst->source || lst->source == NULL))
    {
      /*
      if (e_id == AISL_STREAM_HEADER)
        fprintf(stderr,"FOUND HANDLER %d\n", i);
        */

      /*printf("   catch\n");*/
      switch(e_id)
      {
        /* server events */
        case AISL_SERVER_OPEN:
          i_val = va_arg(vl, aisl_status_t);
          res = ((aisl_server_open_t) lst->cb)( source, i_val );
          break;
        case AISL_SERVER_ERROR:
          i_val = va_arg(vl, aisl_status_t);
          c_ptr = va_arg(vl, char *);
          res = ((aisl_server_error_t) lst->cb)( source, i_val, c_ptr );
          break;

        /* client events */
        case AISL_CLIENT_CONNECT:
          res = ((aisl_client_connect_t) lst->cb)(source, va_arg(vl, void *));
          break;
        case AISL_CLIENT_DISCONNECT:
          res = ((aisl_client_disconnect_t) lst->cb)(source, va_arg(vl, void*));
          aisl_remove_listeners_for( self, source );
          break;
        case AISL_CLIENT_TIMEOUT:
          res = ((aisl_client_timeout_t) lst->cb)(source, va_arg(vl, void *));
          break;

        /* request events */
        case AISL_STREAM_OPEN:
          i_val = va_arg(vl, int);
          c_ptr = va_arg(vl, char *);
          c_ptr2 = va_arg(vl, char *);
          res = ((aisl_stream_open_t) lst->cb)(source, i_val, c_ptr, c_ptr2);
          break;

        case AISL_STREAM_HEADER:
          c_ptr = va_arg(vl, char *);
          c_ptr2 = va_arg(vl, char *);
          res = ((aisl_stream_header_t) lst->cb)(source, c_ptr, c_ptr2);
          break;


        case AISL_STREAM_INPUT:
          /*printf("AISL> raise AISL_STREAM_INPUT\n");*/
          c_ptr = va_arg(vl, char *);
          i_val = va_arg(vl, int );
          res = ((aisl_stream_input_t) lst->cb)(source, c_ptr, i_val);
          break;
        case AISL_STREAM_REQUEST:
          /*printf("AISL> raise AISL_STREAM_REQUEST\n");*/
          buffer_clear( STREAM(source)->buffer, 0);
          res = ((aisl_stream_request_t) lst->cb)(source);
          break;


        case AISL_STREAM_ERROR:
          res = ((aisl_stream_error_t) lst->cb)( source, va_arg(vl, char *));
          break;

        /* response events */
        case AISL_STREAM_OUTPUT:
          res = ((aisl_stream_output_t)lst->cb)(
                  source,
                  va_arg(vl, uint32_t)
                );
          break;
        case AISL_STREAM_CLOSE:
          res = ((aisl_stream_close_t)lst->cb)( source );
          aisl_remove_listeners_for( self, source );
          ((aisl_stream_t) source)->u_ptr=NULL;
          break;

        default:
           res = ((aisl_custom_event_t) lst->cb)(source, vl);
       }
       if (res) break;
    }
  }

  return res;
}



/* -------------------------------------------------------------------------- */
/*
aisl_status_t
aisl_run( int * flags )
{
  aisl_status_t exit_code = AISL_SUCCESS;
  struct timeval timeout;

  while( !(*flags & (1<<0)) )
  {
    exit_code = aisl_run_cycle( gHandle );

    if (exit_code == AISL_IDLE)
    {
      timeout.tv_usec = 300;
      timeout.tv_sec = 0;

       select(0, NULL, NULL, NULL, &timeout);
    }
  }

  return exit_code;
}
*/
/* -------------------------------------------------------------------------- */

#define STAGE_SERVER 0
#define STAGE_CLIENT 1
#define STAGE_DELAY  2

__attribute__ ((visibility ("default") ))
aisl_status_t
aisl_run_cycle( aisl_handle_t self )
{
  int max = self->servers->count+self->clients->count+self->delays->count,
      cnt = 0;


  switch (self->stage)
  {
    case STAGE_SERVER:
      while (self->iterator < self->servers->count )
      {
        server_t srv = (server_t)list_index(self->servers, self->iterator++);
        if ( server_touch(srv) != AISL_IDLE )
          return AISL_SUCCESS;

        if ( ! (++cnt < max) ) return AISL_IDLE;
      }
      if ( ! (self->flags & AISL_HANDLE_HAS_STREAM_LISTENERS) )
        return AISL_IDLE;

      self->iterator = 0;
      self->stage++;


    case STAGE_CLIENT:
      while (self->iterator < self->clients->count )
      {
        int      i   = self->iterator++;
        client_t cli = list_index(self->clients, i);
        bool r = client_touch( cli );

        if (client_is_timeout( cli ) )
          aisl_raise_event( self, cli->server, AISL_CLIENT_TIMEOUT, cli );

        if ( cli->fd == -1 )
        {
          client_free( cli );
          list_remove_index(self->clients, i);
        }

        if (r) return AISL_SUCCESS;

        if ( ! (++cnt < max) ) return AISL_IDLE;
      }
      self->iterator = 0;
      self->stage++;

    case STAGE_DELAY:
      while (self->iterator < self->delays->count )
      {
        int i = self->iterator++;
        delay_t dly = list_index(self->delays, i);

        if( delay_is_expired(dly) )
        {
          if ( ((aisl_delay_timeout_t)  dly->cb)(dly->u_data))
          {
            delay_reset(dly);
          }
          else
            list_remove_index(self->delays, i);

          return AISL_SUCCESS;
        }
        
        if ( ! (++cnt < max) ) return AISL_IDLE;
      }
      self->iterator = 0;
      self->stage = 0;
  }

  return AISL_IDLE;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
const char *
aisl_handle_get_error( aisl_handle_t self )
{
  return self->lastError;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
int
aisl_sleep( aisl_handle_t self, unsigned long usec )
{
  int maxfd=0;
  size_t i;
  struct timeval timeout = {0,usec};

  fd_set  fs;
  FD_ZERO (&fs);

  for (i=0; i<self->servers->count; i++)
  {
    server_t s = list_index(self->servers, i);
    if (s->fd != -1)
    {
      FD_SET(s->fd, &fs);
      if (s->fd > maxfd) maxfd = s->fd;
    }
  }


  for (i=0; i<self->clients->count; i++)
  {
    client_t c = list_index(self->clients, i);
    if (c->fd != -1)
    {
      FD_SET(c->fd, &fs);
      if (c->fd > maxfd) maxfd = c->fd;
    }
  }

  return select(maxfd+1, &fs, NULL, NULL, &timeout);

}




