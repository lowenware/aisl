#ifndef _AISL_SERVER_H_
#define _AISL_SERVER_H_

#include <aisl/status.h>
#include <aisl/handle.h>
#include <arpa/inet.h>

#ifdef __APPLE__

#include <fcntl.h>
#include <sys/ioctl.h>

#endif

/* types -------------------------------------------------------------------- */
struct server
{
  struct sockaddr_in   address;
  aisl_handle_t        owner;
  char               * host;
  int                  fd;
  int                  port;
  int                  flags;
};

typedef struct server * server_t;

#define SERVER(x) ((server_t) x)

/* -------------------------------------------------------------------------- */

server_t
server_new(const char * address, int port);

/* -------------------------------------------------------------------------- */

void
server_free(server_t self);

/* -------------------------------------------------------------------------- */

bool
server_touch(server_t self);

/* -------------------------------------------------------------------------- */

#endif
