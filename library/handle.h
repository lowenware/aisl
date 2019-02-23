#ifndef _AISL_HANDLE_H__
#define _AISL_HANDLE_H__

#include <stdbool.h>
#include <stdarg.h>

#include <aisl/handle.h>

#include <cStuff/list.h>
#include <openssl/ssl.h>
#include "buffer.h"

#define AISL_HANDLE_HAS_STREAM_LISTENERS (1<<8)

/* -------------------------------------------------------------------------- */

struct aisl_handle
{
  list_t       servers;
  list_t       clients;
  list_t       delays;     /* deprecated */
  list_t       listeners;
  list_t       crypters;
  buffer_t     buffer;
  char       * lastError;
  int          iterator;
  int          stage;
  int          flags;
};


/* -------------------------------------------------------------------------- */

aisl_status_t
aisl_set_delay( aisl_handle_t     self,
                aisl_callback_t   cb,
                uint32_t          usec,
                void            * u_data );

/* -------------------------------------------------------------------------- */

bool
aisl_raise_event_vl( aisl_handle_t   self,
                     void          * source,
                     aisl_event_t    e_id,
                     va_list         vl ); 

/* -------------------------------------------------------------------------- */


SSL_CTX *
aisl_get_ssl_ctx( aisl_handle_t self, const char * server_name );

/* -------------------------------------------------------------------------- */

void
aisl_remove_listeners_for( aisl_handle_t self, void * source );

/* -------------------------------------------------------------------------- */

#endif
