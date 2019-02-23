#ifndef _AISL_CLIENT_H_
#define _AISL_CLIENT_H_

#include <time.h>
#include <arpa/inet.h>

#include <aisl/aisl.h>
#include <aisl/http.h>
#include <cStuff/list.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "server.h"

#define CLIENT_FLAG_KEEPALIVE (1<<0)
#define CLIENT_FLAG_HANDSHAKE (1<<1)


struct client
{
  struct sockaddr_in   address;
  server_t             server;
  int                  fd;

  int                  next_id; /* server id generator (even, starts from 2) */
  int                  istream; /* input stream id */
  int                  ostream; /* output stream id */
  list_t               streams;
  SSL                * ssl;

  time_t               timestamp;
  aisl_http_version_t  protocol;
  int                  flags;
};

typedef struct client * client_t;

#define CLIENT(x) ((client_t)x)

/* constructor -------------------------------------------------------------- */

aisl_status_t
client_accept(client_t * self, server_t server);

/* destructor --------------------------------------------------------------- */

void
client_free(client_t self);

/* all regular client routines. return true if something happened ----------- */

bool
client_touch(client_t self);

/* check if communication time with client is expired ----------------------- */

bool
client_is_timeout(client_t self);

/* -------------------------------------------------------------------------- */

void
client_close(client_t self);

/* -------------------------------------------------------------------------- */
#endif
