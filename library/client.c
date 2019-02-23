#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <aisl/aisl.h>
#include "client.h"
#include "stream.h"
#include "parser.h"
#include "globals.h"
#include "handle.h"

#ifndef OUTPUT_BUFFER_SIZE
#define OUTPUT_BUFFER_SIZE 4096
#endif

/* -------------------------------------------------------------------------- */

void
client_close(client_t self)
{
  close(self->fd);
  /* will provide double free
  if (self->ssl)
    SSL_free(self->ssl);
    */
  shutdown(self->fd, SHUT_RDWR);
  self->fd=-1;
}

/* -------------------------------------------------------------------------- */

static bool
client_input(client_t self)
{
  int                l;
  parser_status_t    p_status = PARSER_PENDING;
  char              *ptr;
  buffer_t           buffer = self->server->owner->buffer;

  stream_t s = list_index(self->streams, self->istream);


  if (self->ssl)
  {
    if (self->flags & CLIENT_FLAG_HANDSHAKE)
    {
      if ( (l = SSL_accept(self->ssl)) != 1 )
      {
        l = SSL_get_error(self->ssl, l);

        if (l == SSL_ERROR_WANT_READ || l == SSL_ERROR_WANT_WRITE)
          return false;

        client_close(self);
        fprintf(stderr, "SSL handshake fail: %s\n", ERR_error_string(l, NULL));
        return true;
      }

      self->flags ^= CLIENT_FLAG_HANDSHAKE;
    }

    l = SSL_read(self->ssl, buffer->data, buffer->size) ;
  }
  else
    l = recv(    self->fd,  buffer->data, buffer->size, 0);

  if (l>0)
  {
    if( buffer_add(s->buffer, buffer->data, l) == BUFFER_EOB )
    {
      client_close(self);
      return true;
    }

    ptr = s->buffer->data;
    l   = s->buffer->size;

    /* parse next data chunk */
    while ( p_status == PARSER_PENDING )
    {
      switch(s->state)
      {
        case STREAM_REQUEST_METHOD:
          p_status = parse_request_method(self, &ptr, &l);
          break;
        case STREAM_REQUEST_PATH:
          p_status = parse_request_path(self, &ptr, &l);
          break;
        case STREAM_REQUEST_PROTOCOL:
          p_status = parse_request_protocol(self, &ptr, &l);
          break;

        case STREAM_REQUEST_HEADER_KEY:
          p_status = parse_request_header_key(self, &ptr, &l);
          break;
        case STREAM_REQUEST_HEADER_VALUE:
          p_status = parse_request_header_value(self, &ptr, &l);
          break;

        case STREAM_REQUEST_CONTENT:
          p_status = parse_request_content(self, &ptr, &l);
          break;

        default:
          p_status = PARSER_FINISHED;
            /* this is error actually */

      }
    }

    if (p_status == PARSER_FAILED)
    {
      /* reply Bad Request here */
      client_close(self);
    }
    else if (l)
    {
      buffer_shift(s->buffer, s->buffer->size-l); /* reset buffer */
    }
    /*
    else
      buffer_clear(s->buffer, 0);*/

    return true;
  }
  else if (l<0)
  {
    if (self->ssl)
    {
      if (SSL_get_error(self->ssl, l) == SSL_ERROR_WANT_READ)
        return false;
    }
    else
    {
      if(errno == EWOULDBLOCK)
        return false;
    }
  }

  /* both: client disconnect + on read error  */
  /* todo: raise client error here */
  client_close(self);

  return true;
}

static bool
client_output(client_t self)
{
  if (self->fd < 0)
  {
    fprintf(stderr, "[aisl] assertion !(client->fd<0) failed\n");
    return true;
  }

  stream_t s;
  int      l;

  s = list_index(self->streams, self->ostream);

  /*
  if (!s->c_length_unknown && s->buffer && s->buffer->len)
    buffer_move(gBuffer, s->buffer);
  */

  /* while stream is not flushed, we should raise event */
  if(s->flags & STREAM_FLAG_OUTPUT_CHUNKED)
  {
    /* in case of chunked output ( subscription for AISL_STREAM_OUTPUT event )
     * stream buffer will be initialized with OUTPUT_BUFFER_SIZE size, but
     * buffer->size will be used to carry amount of stored bytes
     * */
    size_t bsz = s->buffer->size;

    if (bsz < OUTPUT_BUFFER_SIZE)
    {
      if (buffer_clear(s->buffer, OUTPUT_BUFFER_SIZE) == 0)
        return false;

      s->buffer->size = bsz;
      bsz = OUTPUT_BUFFER_SIZE;
    }

    if ( (l = bsz - s->buffer->size) > OUTPUT_BUFFER_SIZE / 2 )
      aisl_raise_event( self->server->owner, s, AISL_STREAM_OUTPUT, l);
  }

  if (s->buffer->size == 0)
    return false;

  l = (self->ssl) ?
        SSL_write(self->ssl, s->buffer->data, s->buffer->size) :
        send(     self->fd,  s->buffer->data, s->buffer->size, 0);

  if (l > 0)
  {
    buffer_shift(s->buffer, l);
    if (s->state == STREAM_RESPONSE_READY && /* flushed */
        s->buffer->size == 0) /* all sent */
    {
      buffer_clear(s->buffer, 0);

      /* data has been sent */
      /*
      if (self->protocol == AISL_HTTP_2_0)
      {

      }
      else*/
      if (self->flags & CLIENT_FLAG_KEEPALIVE)
      {
        list_remove(self->streams, s);
        stream_free(s);

        s = stream_new((struct sockaddr_in  *) self, self->next_id++, STREAM_REQUEST_METHOD );
        list_append(self->streams, s);
      }
      else
      {
        client_close(self);
      }
    }
    return true;
  }

  /* l < 0 */
  if (self->ssl)
  {
    if ( SSL_get_error(self->ssl, l) == SSL_ERROR_WANT_WRITE )
      return false;
  }
  else
  {
    if (errno == EWOULDBLOCK)
      return false;
  }

  client_close(self);

  return true;
}

bool
client_touch(client_t self)
{
  bool     result = false;
  stream_t s;

  /* input */
  s = list_index(self->streams, self->istream);

  if ((self->protocol==AISL_HTTP_2_0 || s->state<STREAM_REQUEST_READY) &&
      (client_input(self)) ) result = true;

  /* output */
  s = list_index(self->streams, self->ostream);

  if ( s->flags & (STREAM_FLAG_OUTPUT_READY | STREAM_FLAG_OUTPUT_CHUNKED) )
    result = client_output(self);

  /* update timestamp */
  if (result)
    self->timestamp = time(NULL);

  return result;
}


/* constructor -------------------------------------------------------------- */

static client_t
client_new( int fd, struct sockaddr_in * addr )
{
  client_t   self;
  stream_t   stream;

  if ( !(self = calloc(1, sizeof(struct client))) )
    goto finally;

  memcpy(&self->address, addr, sizeof(struct sockaddr_in));

  self->fd         = fd;
  self->next_id    = 2;
  /*
  self->istream    = 0;
  self->ostream    = 0;
   * UTPUT
  */
  self->protocol   = AISL_HTTP_1_0;
  self->timestamp  = time(NULL);
  self->flags      = CLIENT_FLAG_KEEPALIVE | CLIENT_FLAG_HANDSHAKE;

  if ( !(self->streams = list_new(AISL_MIN_STREAMS)) )
    goto except;

  if ( !(stream = stream_new((struct sockaddr_in *)self, 0, STREAM_REQUEST_METHOD)) )
    goto e_stream;

  if (list_append(self->streams, stream)  == -1)
    goto e_append;

  goto finally;

e_append:
  stream_free(stream);

e_stream:
  list_free(self->streams, NULL);

except:
  free(self);
  self=NULL;

finally:
  return self;
}

aisl_status_t
client_accept(client_t * p_self, server_t server)
{
  aisl_status_t      result;
  const char       * e_detail = NULL;
  int                fd;
  struct sockaddr_in addr;
  socklen_t          len = sizeof(struct sockaddr_in);
  SSL              * ssl = NULL;
  SSL_CTX          * ssl_ctx;

  *p_self = NULL;

  if ( (fd = accept(server->fd, (struct sockaddr *) &addr, &len)) < 0 )
  {
    if (errno != EWOULDBLOCK)
    {
      result = AISL_SYSCALL_ERROR;
      e_detail = strerror(errno);
      goto raise;
    }

    result = AISL_IDLE;
    goto finally;
  }

  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1)
  {
    result = AISL_SYSCALL_ERROR;
    e_detail = strerror(errno);
    goto raise;
  }

  flags |= O_NONBLOCK;

  if (fcntl(fd, F_SETFL, flags) != 0)
  {
    result = AISL_SYSCALL_ERROR;
    e_detail = strerror(errno);
    goto raise;
  }

  if (server->flags & AISL_FLAG_SSL)
  {
    if ( !(ssl_ctx = aisl_get_ssl_ctx( server->owner, NULL )) )
      goto except;

    if ( !(ssl = SSL_new(ssl_ctx)) )
    {
      e_detail = "SSL_new";
      result = AISL_EXTCALL_ERROR;
      goto except;
    }

    SSL_set_fd(ssl, fd);

  }
  else
    ssl = NULL;

  if ( !(*p_self = client_new(fd, &addr)) )
  {
    result = AISL_MALLOC_ERROR;
    e_detail = "client_t";
    goto raise;
  }

  (*p_self)->server = server;
  (*p_self)->ssl    = ssl;
  result            = AISL_SUCCESS;

  goto finally;

except:
  close(fd);
  if (ssl)
    SSL_free(ssl);

raise:
  aisl_raise_event(
    server->owner,
    server,
    AISL_SERVER_ERROR,
    server->flags,
    e_detail
  );

finally:
  return result;
}

/* destructor --------------------------------------------------------------- */

void
client_free(client_t self)
{
  if (self->fd > -1)
    close(self->fd);

  if (self->ssl)
    SSL_free(self->ssl);

  list_free(self->streams, (list_destructor_t)stream_free);

  aisl_raise_event(
    self->server->owner,
    self->server,
    AISL_CLIENT_DISCONNECT,
    self
  );

  free(self);
}

/* check if communication time with client is expired ----------------------- */

bool
client_is_timeout(client_t self)
{
  bool result = false;
  stream_t s;
  if (self->protocol == AISL_HTTP_2_0)
  {

  }
  else
  {
    s = list_index(self->streams, self->istream);
    if ( (s->state < STREAM_REQUEST_READY) && /* still waiting for data */
         (time(NULL)-self->timestamp > AISL_MAX_CLIENT_SILENCE) ) result=true;
  }

  if (result)
    client_close(self);

  return result;
}

/* -------------------------------------------------------------------------- */
