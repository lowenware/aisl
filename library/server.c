#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <cStuff/str-utils.h>


#include "server.h"
#include "handle.h"
#include "client.h"
#include "globals.h"

/* -------------------------------------------------------------------------- */

static bool
_set_addres_from_host_and_port( struct sockaddr_in * sa,
                                const char         * host,
                                int                  port )
{
  int               rs;
  struct addrinfo * ai;

  memset(sa, 0, sizeof( struct sockaddr_in ));
  sa->sin_family = AF_INET;

  rs = getaddrinfo(host, NULL, NULL, &ai);

  if(rs != 0)
  {
    return false;
  }

  sa->sin_addr.s_addr=((struct sockaddr_in*)(ai->ai_addr))->sin_addr.s_addr;
  sa->sin_port       =htons(port);

  freeaddrinfo(ai);

  return true;
}


/* -------------------------------------------------------------------------- */

static void
server_close(server_t self)
{
  close(self->fd);
  self->fd=-1;
}

/* -------------------------------------------------------------------------- */


static aisl_status_t
server_open(server_t self)
{

  aisl_status_t result = AISL_SUCCESS;

  int       s_opt  = 1;

  if (!_set_addres_from_host_and_port(&self->address, self->host, self->port))
    return AISL_EXTCALL_ERROR;

  self->fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  
  if (self->fd != -1)
  {
    setsockopt(
      self->fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&s_opt, sizeof(int)
    );

#ifdef __APPLE__

    int on = 1;

    ioctl(self->fd, FIONBIO, (char *)&on);
    fcntl(self->fd, F_SETFL, fcntl(self->fd, F_GETFL) | O_NONBLOCK);

#endif

    if (bind( self->fd, 
              (struct sockaddr *) &self->address,
              sizeof(struct sockaddr_in) )==0)
    {
      if (listen(self->fd, SOMAXCONN) == 0)
        return result;
    }

    server_close(self);
  }

  result = AISL_SYSCALL_ERROR;

  return result;
}


/* -------------------------------------------------------------------------- */

bool
server_touch(server_t self)
{
  aisl_status_t result;
  client_t cli;

  if (self->fd == -1)
  {
    result = server_open(self);
    if (result == AISL_SUCCESS)
      aisl_raise_event(
        self->owner,
        self,
        AISL_SERVER_OPEN,
        self->flags
      );
    else
      aisl_raise_event(
        self->owner,
        self,
        AISL_SERVER_ERROR,
        self->flags,
        strerror(errno)
      );

    return result;
  }

  result = client_accept(&cli, self);

  if (result == AISL_SUCCESS)
  {
    if (list_append(self->owner->clients, cli) == -1)
    {
      client_free(cli);
      result = AISL_MALLOC_ERROR;
    }
    else
      aisl_raise_event(self->owner, self, AISL_CLIENT_CONNECT, cli);

  }

  return result;
}

/* -------------------------------------------------------------------------- */

server_t
server_new(const char * address, int port)
{
  server_t self;
  
  if ( (self = calloc(1, sizeof(struct server))) != NULL )
  {
    self->fd = -1;
    self->port = port;
    if ( !(self->host = str_copy(address)) )
    {
      free(self);
      self = NULL;
    }
  }

  return self;
}

/* -------------------------------------------------------------------------- */

void
server_free(server_t self)
{
  if (self)
  {
    if (self->fd > -1)
      server_close(self);

    if (self->host)
      free(self->host);

    free(self);
  }
}


/* -------------------------------------------------------------------------- */

