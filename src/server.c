#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <openssl/ssl.h>

#ifdef __APPLE__
#include <sys/ioctl.h>
#endif

#include "debug.h"
#include "str-utils.h"
#include "instance.h"
#include "client.h"
#include "server.h"


/**
 * @brief Creates TCP server socket, binds to address and starts to listen.
 * @param server a pointer to #AislServer instance.
 * @return #AislStatus code.
 */
static AislStatus
aisl_server_open(AislServer server)
{
	int fd, s_opt  = 1;

	fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

	if (fd != -1) {
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&s_opt,
		  sizeof (int));

		#ifdef __APPLE__
		ioctl(fd, FIONBIO, (char *)&s_opt);
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
		#endif

		s_opt = sizeof (struct sockaddr_in);

		if (bind(fd, (struct sockaddr *) &server->address, s_opt) == 0) {
			if (listen(fd, SOMAXCONN) == 0) {
				server->fd = fd;
				return AISL_SUCCESS;
			}
		}
		close(fd);
	}

	return AISL_SYSCALL_ERROR;
}


/**
 * @brief Tries to accept a new client.
 * @param server a pointer to #AislServer instance.
 * @param p_client a pointer to store #AislClient instance pointer.
 * @return #AislStatus code.
 */
static AislStatus
aisl_server_accept(AislServer server, AislClient *p_client )
{
	int                fd;
	struct sockaddr_in addr;
	socklen_t          len = sizeof (struct sockaddr_in);

	fd = accept(server->fd, (struct sockaddr *)&addr, &len);

	if (fd != -1) {
		int flags;

		DPRINTF("accepted fd=%d", fd);

		if ((flags = fcntl(fd, F_GETFL, 0)) != -1) {
			flags |= O_NONBLOCK;

			if (fcntl(fd, F_SETFL, flags) == 0) {
				return (!(*p_client = aisl_client_new(server, fd, &addr))) ?
					 AISL_MALLOC_ERROR : AISL_SUCCESS;
			}
		}
		close(fd);
	} else if (errno == EWOULDBLOCK) {
		return AISL_IDLE;
	}
	return AISL_SYSCALL_ERROR;
}

/* Library Level ------------------------------------------------------------ */

AislStatus
aisl_server_touch(AislServer server, AislClient *p_client)
{
	AislStatus result;

	if (server->fd == -1) {
		if ((result = aisl_server_open(server)) != AISL_IDLE) {
			aisl_raise(server->instance, server, ((result == AISL_SUCCESS) ?
			  AISL_EVENT_SERVER_READY : AISL_EVENT_SERVER_ERROR), result);
		}
	} else {
		result = aisl_server_accept(server, p_client);
	}
	return result;
}


int
aisl_server_get_socket(AislServer server)
{
	return server->fd;
}


AislServer
aisl_server_new(const struct aisl_cfg_srv *cfg_srv, AislInstance instance)
{
	AislServer server;

	if ((server = calloc(1, sizeof (struct aisl_server))) != NULL) {
		server->instance = instance;
		server->fd = -1;
		server->address.sin_family = AF_INET;
		server->address.sin_addr.s_addr = inet_addr(cfg_srv->host);
		server->address.sin_port = htons(cfg_srv->port);
		server->ssl = cfg_srv->secure;
	}
	return server;
}


void
aisl_server_free(AislServer server)
{
	if (server) {
		if ( server->fd != -1) {
			close(server->fd);
			server->fd=-1;
		}
		free(server);
	}
}


/* API Level ---------------------------------------------------------------- */


__attribute__ ((visibility ("default") ))
void
aisl_server_get_address(AislServer server, struct sockaddr_in *address)
{
	memcpy(address, &server->address, sizeof (struct sockaddr_in));
}


#if AISL_WITH_SSL == 1

__attribute__ ((visibility ("default") ))
bool
aisl_server_get_ssl(AislServer server)
{
	return server->ssl;
}

#endif
