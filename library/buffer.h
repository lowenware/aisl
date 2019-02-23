#ifndef _AISL_BUFFER_H_
#define _AISL_BUFFER_H_

#include <sys/types.h>

/* -------------------------------------------------------------------------- */

struct buffer
{
  char    * data;
  size_t    size;
};

typedef struct buffer * buffer_t;

#define BUFFER_EOB ( ~0 )

/* -------------------------------------------------------------------------- */

buffer_t
buffer_new( size_t initial_size );

/* -------------------------------------------------------------------------- */

void
buffer_free( buffer_t self );

/* -------------------------------------------------------------------------- */

size_t
buffer_insert( buffer_t self, size_t offset, const char * data, size_t size );

/* -------------------------------------------------------------------------- */
size_t
buffer_add( buffer_t self, const char * data, size_t size );

/* -------------------------------------------------------------------------- */

size_t
buffer_clear( buffer_t self, size_t to_alloc );

/* -------------------------------------------------------------------------- */

size_t
buffer_shift( buffer_t self, size_t size );

/* -------------------------------------------------------------------------- */

#endif
