#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>

#include <pilot_atk.h>
#include <pilot_ntk.h>
#include "pilot_socket.h"

static int
_pilot_server_udp_accept( struct pilot_server_udp *thiz);
static int
_pilot_server_udp_wait( struct pilot_server_udp *thiz);
static int
_pilot_socket_udp_connect( struct pilot_socket_udp *thiz);

struct pilot_server_udp *
pilot_server_udp_create(struct pilot_application *application, int port)
{
	struct pilot_server_udp *thiz;
	thiz = malloc(sizeof(*thiz));
	struct pilot_socket *socket = &thiz->socket;

	LOG_DEBUG("");
	_pilot_socket_init(socket, application, PILOT_SERVER_UDP);
	thiz->port = port;
	thiz->address = NULL;

	_pilot_socket_open((struct pilot_socket *)thiz);
	pilot_connect(socket->connector, dispatch_events, thiz, _pilot_server_udp_accept);

	_pilot_server_udp_wait(thiz);
	return thiz;
}

void
pilot_server_udp_destroy(struct pilot_server_udp *thiz)
{
	_pilot_socket_destroy(&thiz->socket);
	if (thiz->address != NULL)
		free(thiz->address);
	free(thiz);
}

struct pilot_socket_udp *
_pilot_socket_udp_create(struct pilot_application *application, char *address, int port)
{
	struct pilot_socket_udp *thiz;
	thiz = malloc(sizeof(*thiz));

	_pilot_socket_init(&thiz->socket, application, PILOT_SOCKET_UDP);
	thiz->port = port;
	if (address != NULL)
	{
		thiz->address = malloc(strlen(address)+1);
		strcpy(thiz->address, address);
	}

	return thiz;
}

struct pilot_socket_udp *
pilot_socket_udp_create(struct pilot_application *application, char *address, int port)
{
	struct pilot_socket_udp *thiz;
	struct pilot_socket *socket;
	thiz = _pilot_socket_udp_create(application, address, port);

	LOG_DEBUG("");
	_pilot_socket_open((struct pilot_socket *)thiz);
	_pilot_socket_udp_connect(thiz);
	pilot_connect(thiz->socket.connector, disconnect, thiz, _pilot_socket_disconnect);
	return thiz;
}

void
pilot_socket_udp_destroy(struct pilot_socket_udp *thiz)
{
	_pilot_socket_destroy(&thiz->socket);
	free(thiz);
}

static int
_pilot_server_udp_wait( struct pilot_server_udp *thiz)
{
	struct sockaddr_in address;
	int broadcast = 0;

	LOG_DEBUG("");
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(thiz->port);


	if (thiz->address == NULL)
		address.sin_addr.s_addr = htonl(INADDR_ANY);
	else if(!strcmp(thiz->address, BROADCAST))
	{
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		broadcast = 1;
	}
	else if(!strcmp(thiz->address, MULTICAST))
	{
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		broadcast = 1;
	}
	else
		inet_aton(thiz->address, &address.sin_addr);


	if (broadcast)
		LOG_DEBUG("broadcast");
	if (setsockopt(thiz->socket.connector->fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
	{
		LOG_DEBUG("error broadcast opt: %s", strerror(errno));
		close(thiz->socket.connector->fd);
		thiz->socket.connector->fd = -1;
		return -errno;
	}
	return _pilot_socket_wait(&thiz->socket, (struct sockaddr *)&address, sizeof(address));
}

static int
_pilot_server_udp_accept( struct pilot_server_udp *thiz)
{
	LOG_DEBUG("");
	pilot_emit(thiz, connection, (struct pilot_socket *)thiz);
	return 0;
}

static int
_pilot_socket_udp_connect( struct pilot_socket_udp *thiz)
{
	struct sockaddr_in address = {0};
	int broadcast = 0;

	LOG_DEBUG("");
	address.sin_family = AF_INET;
	address.sin_port = htons(thiz->port);
	if (thiz->address == NULL)
		address.sin_addr.s_addr = htonl(INADDR_ANY);
	else if(!strcmp(thiz->address, BROADCAST) || !strcmp(thiz->address, MULTICAST))
	{
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		broadcast = 1;
	}
	else
	{
		int ret;
		ret = inet_aton(thiz->address, &address.sin_addr);
		//ret = inet_pton(address.sin_family, thiz->address, (void *)&address.sin_addr.s_addr);
	}

	if (broadcast)
		LOG_DEBUG("broadcast");
	if (setsockopt(thiz->socket.connector->fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
	{
		LOG_DEBUG("error broadcast opt: %s", strerror(errno));
		close(thiz->socket.connector->fd);
		thiz->socket.connector->fd = -1;
		return -errno;
	}
	return _pilot_socket_connect(&(thiz->socket), (struct sockaddr *)&address, sizeof(address));
}

int
_pilot_socket_udp_read(struct pilot_socket_udp *thiz, char *buff, int size)
{
	struct pilot_socket *socket = &thiz->socket;
	return read(socket->connector->fd, buff, size);
}

int
_pilot_socket_udp_write(struct pilot_socket_udp *thiz, char *buff, int size)
{
	struct pilot_socket *socket = &thiz->socket;
	return write(socket->connector->fd, buff, size);
}

#ifdef TEST_SERVER
int
main(int argc, char **argv)
{
	struct pilot_application *application;
	application = pilot_application_create(argc, argv);
	struct pilot_server_udp *server;
	server = pilot_server_udp_create(application, 8080);

	pilot_application_run(application);

	pilot_server_udp_destroy(server);
	pilot_application_destroy(application);
	return 0;
}

#endif
