#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "buffer.h"

/* -------------------------------------------------------------------------- */

buffer_t
buffer_new( size_t initial_size )
{
  buffer_t self;

  if ( (self = calloc(1, sizeof(struct buffer))) != NULL)
  {
    if ( (self->size = initial_size) > 0 )
    {
      if ( !(self->data = calloc( initial_size+1, sizeof(char) )) )
      {
        free(self);
        self = NULL;
      }
    }
  }

  return self;
}

/* -------------------------------------------------------------------------- */

void
buffer_free( buffer_t self )
{
  if (self->data)
    free(self->data);

  free(self);
}

/* -------------------------------------------------------------------------- */

size_t
buffer_add( buffer_t self, const char * data, size_t size )
{
  size_t result = size+self->size;

  char * ptr;

  if ( (ptr = realloc(self->data, result+1)) != NULL )
  {
    memcpy( &ptr[self->size], data, size );
    ptr[ result ] = 0;
    self->data = ptr;
    self->size = result;
  }
  else
    result = BUFFER_EOB;

  return result;
}

/* -------------------------------------------------------------------------- */

size_t
buffer_clear( buffer_t self, size_t to_alloc )
{
  char * data;

  self->size = 0;

  if (to_alloc)
  {
    if ( (data = realloc(self->data, to_alloc)) != NULL )
    {
      self->data = data;
      return to_alloc;
    }
  }

  free(self->data);
  self->data = NULL;

  return 0;
}

/* -------------------------------------------------------------------------- */

size_t
buffer_shift( buffer_t self, size_t size )
{
  size_t result;

  if (size && !(size > self->size))
  {
    result = self->size - size;
    memmove(self->data, &self->data[size], result);
    self->size = result;
  }
  else
    result = BUFFER_EOB;

  return result;
}

/* -------------------------------------------------------------------------- */

size_t
buffer_insert( buffer_t self, size_t offset, const char * data, size_t size )
{
  size_t result = size + self->size;

  char * ptr;

  if ( (ptr = realloc(self->data, result+1)) != NULL )
  {
    memmove( &ptr[offset+size], &ptr[offset], self->size - offset );
    memcpy( &ptr[offset], data, size );
    ptr[ result ] = 0;
    self->data = ptr;
    self->size = result;
  }
  else
    result = BUFFER_EOB;

  return result;

}

/* -------------------------------------------------------------------------- */
