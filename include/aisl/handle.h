#ifndef _AISL_HANDLE_H_
#define _AISL_HANDLE_H_

#include <aisl/status.h>
#include <aisl/event.h>

/* -------------------------------------------------------------------------- */

#define AISL_FLAG_SSL (1<<0)

/* -------------------------------------------------------------------------- */

typedef struct aisl_handle * aisl_handle_t;


/* -------------------------------------------------------------------------- */

aisl_handle_t
aisl_handle_new(size_t min_clients, size_t buffer_size);

/* -------------------------------------------------------------------------- */

aisl_handle_t
aisl_handle_from_stream( aisl_stream_t s );

/* -------------------------------------------------------------------------- */

void
aisl_handle_free( aisl_handle_t self );

/* -------------------------------------------------------------------------- */

aisl_status_t
aisl_bind( aisl_handle_t self, const char * address, int port, int flags );

/* -------------------------------------------------------------------------- */

aisl_status_t
aisl_set_ssl( aisl_handle_t self, const char * server_name,
                                  const char * key_file,
                                  const char * crt_file );

/* -------------------------------------------------------------------------- */

aisl_status_t
aisl_set_callback( aisl_handle_t    self,
                   void           * source,
                   aisl_event_t     e_id,
                   aisl_callback_t  cb );

/* -------------------------------------------------------------------------- */

bool
aisl_raise_event( aisl_handle_t   self,
                  void          * source,
                  aisl_event_t    e_id,
                                  ... ); 

/* -------------------------------------------------------------------------- */

aisl_status_t
aisl_run_cycle( aisl_handle_t self );

/* -------------------------------------------------------------------------- */

const char *
aisl_handle_get_error( aisl_handle_t self );

/* -------------------------------------------------------------------------- */

int
aisl_sleep( aisl_handle_t self, unsigned long usec );

/* -------------------------------------------------------------------------- */

#endif
