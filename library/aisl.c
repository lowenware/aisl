#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <sys/time.h>

#include <aisl/aisl.h>
#include <cStuff/list.h>
#include <cStuff/str-utils.h>

#include "handle.h"
#include "client.h"
#include "server.h"
#include "globals.h"
#include "stream.h"
#include "buffer.h"


/* Globals ------------------------------------------------------------------ */

#define AISL_TERMINATED 1

aisl_handle_t gHandle = NULL;


/* DEPRECATED --------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_status_t
aisl_init(  )
{
  gHandle = aisl_handle_new( AISL_MIN_CLIENTS, AISL_BUFFER_SIZE );

  return (gHandle != NULL) ? AISL_SUCCESS : AISL_MALLOC_ERROR;
}

/* DEPRECATED --------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
void
aisl_release()
{
  aisl_handle_free(gHandle);
}

/* DEPRECATED --------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_status_t
aisl_select(const char * address, int port)
{
  return aisl_bind(gHandle, address, port, 0);
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_status_t
aisl_run( int * flags )
{
  aisl_status_t exit_code = AISL_SUCCESS;
  struct timeval timeout = {0,0};

  while( !(*flags & AISL_TERMINATED) )
  {
    if ( (exit_code = aisl_run_cycle( gHandle )) == AISL_IDLE )
    {
      timeout.tv_usec = 300;
      timeout.tv_sec = 0;

      select(0, NULL, NULL, NULL, &timeout);
    }
  }

  return exit_code;
}

/* Event calls -------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_status_t
aisl_delay( aisl_callback_t cb, uint32_t usec, void * u_data )
{
  return aisl_set_delay(gHandle, cb, usec, u_data);
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
aisl_status_t
aisl_listen( void * source, aisl_event_t e_id, aisl_callback_t cb )
{
  return aisl_set_callback( gHandle, source, e_id, cb );
}

/* -------------------------------------------------------------------------- */


__attribute__ ((visibility ("default") ))
bool
aisl_raise( void *source, aisl_event_t e_id, ... )
{
  bool result;
  va_list vl;
  va_start(vl, e_id);
  result = aisl_raise_event_vl( gHandle, source, e_id, vl);
  va_end(vl);
  return result;
}

__attribute__ ((visibility ("default") ))
const char *
aisl_header_get(aisl_stream_t stream, const char *key)
{
  int i;
  if (STREAM(stream)->headers)
  {
    for (i=0; i< STREAM(stream)->headers->count; i++)
    {
      if (str_cmpi(((pair_t)list_index(STREAM(stream)->headers,i))->key,key)==0)
        return ((pair_t) list_index(STREAM(stream)->headers,i))->value;
    }
  }

  return NULL;
}

/* response functions ------------------------------------------------------- */

static char *
get_response_begin(stream_t stream)
{
  char * r;
  client_t cli = CLIENT(ASTREAM(stream)->client);

  r = str_printf(
        "%s %d %s\r\n"
        "Server: AISL\r\n"
        "Connection: %s\r\n\r\n",
        aisl_http_version_to_string(cli->protocol),
        stream->response,
        aisl_http_response_to_string(stream->response),
        ((cli->flags & CLIENT_FLAG_KEEPALIVE) ? "keep-alive" : "close")
      );

  return r;
}

__attribute__ ((visibility ("default") ))
aisl_status_t
aisl_response(aisl_stream_t stream, aisl_http_response_t  status_code,
                                    const char           *content_type,
                                    uint32_t              content_length)
{
  char  * pch;
  int     l;


  /* check if those headers were already sent */
  if (STREAM(stream)->state > STREAM_REQUEST_READY) return AISL_IDLE;

  STREAM(stream)->response = status_code;
  STREAM(stream)->c_type   = content_type;
  STREAM(stream)->c_length = content_length;

  if ( !(pch = get_response_begin(STREAM(stream))) )
    return AISL_MALLOC_ERROR;

  l = strlen(pch);
  STREAM(stream)->c_offset = l-2;

  buffer_clear(STREAM(stream)->buffer, content_length);

  l = buffer_add( STREAM(stream)->buffer, pch, l );
  free(pch);

  if (l == BUFFER_EOB) return AISL_MALLOC_ERROR;

  if (content_length)
  {
    if (!aisl_header_printf( stream, "Content-Length", "%u", content_length ))
      return AISL_MALLOC_ERROR;
  }

  if (content_type)
  {
    if (!aisl_header( stream, "Content-Type", content_type ))
      return AISL_MALLOC_ERROR;
  }

  STREAM(stream)->state = STREAM_RESPONSE_HEADER;

  return AISL_SUCCESS;
}

__attribute__ ((visibility ("default") ))
aisl_status_t
aisl_flush(aisl_stream_t stream)
{
  stream_t s = STREAM(stream); 
  if ( ! s->c_length )
  {
    s->c_length = s->buffer->size - s->c_offset - 2;
    if (!aisl_header_printf(stream, "Content-Length", "%u", s->c_length))
      return AISL_MALLOC_ERROR;
  }

  /*
  fprintf(stdout, "(%lu bytes)------->\n", STREAM(stream)->buffer->size);
  fwrite(STREAM(stream)->buffer->data, 1, STREAM(stream)->buffer->size, stdout);
  fprintf(stdout, "<------\n");
  */
  s->state = STREAM_RESPONSE_READY;

  s->flags |= STREAM_FLAG_OUTPUT_READY;

  return AISL_SUCCESS;
}

/* header functions --------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
int
aisl_header(aisl_stream_t stream, const char *key, const char *value)
{
  int ret;
  char * pch;

  if ( (pch = str_printf("%s: %s\r\n", key, value)) != NULL )
  {
    ret = strlen(pch);
    if ( buffer_insert(
           STREAM(stream)->buffer,
           STREAM(stream)->c_offset,
           pch,
           ret
         ) == BUFFER_EOB )
    {
      ret = -1;
    }
    else
      STREAM(stream)->c_offset += ret;

    free(pch);
  }
  else
    ret = -1;

  return ret;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
int
aisl_header_printf(aisl_stream_t stream, const char *key,
                                         const char *f_value, ...)
{
  int ret;

  va_list arg;
  va_start(arg, f_value);

  ret = aisl_header_vprintf(stream, key, f_value, arg);

  va_end(arg);

  return ret;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
int
aisl_header_vprintf(aisl_stream_t stream, const char *key,
                                          const char *format,
                                          va_list     args)
{
  int ret;
  char * value;

  if ( (value = str_vprintf(format, args)) != NULL )
  {
    ret = aisl_header( stream, key, value );
    free(value);
  }
  else
    ret = -1;

  return ret;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
int
aisl_printf(aisl_stream_t stream, const char *format, ...)
{
  va_list arg;
  va_start(arg, format);

  int result = aisl_vprintf(stream, format, arg);

  va_end(arg);

  /* No need to update length there, because vprintf do that
   * 
   * if (STREAM(stream)->c_length_unknown)
    STREAM(stream)->c_length += result;
    */


  return result;

}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
int
aisl_vprintf(aisl_stream_t stream, const char *format, va_list args)
{
  int    result;
  char * r;

  if ( (r = str_vprintf(format, args)) != NULL)
  {
    result = strlen(r);
    if (buffer_add(STREAM(stream)->buffer, r, result) == BUFFER_EOB)
      result = -1;

    free(r);
  }
  else
    result = -1;

  return result;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
int
aisl_write(aisl_stream_t stream, const char *data, int d_len)
{
  if (d_len < 0)
    d_len = strlen(data);

  if (buffer_add(STREAM(stream)->buffer, data, d_len) == BUFFER_EOB)
    d_len = -1;

  return d_len;
}

/* -------------------------------------------------------------------------- */

__attribute__ ((visibility ("default") ))
int
aisl_puts(const char *str, aisl_stream_t stream)
{
  return aisl_write( stream, str, strlen(str));
}

/* -------------------------------------------------------------------------- */

