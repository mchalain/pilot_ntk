#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

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
_pilot_server_tcp_accept( struct pilot_server_tcp *thiz);
static int
_pilot_server_tcp_wait( struct pilot_server_tcp *thiz);
static int
_pilot_socket_tcp_connect( struct pilot_socket_tcp *thiz);
static int
_pilot_socket_tcp_disconnect( struct pilot_socket *thiz);
static int
_pilot_socket_tcp_read(struct pilot_socket *thiz, char *buff, int size);
static int
_pilot_socket_tcp_write(struct pilot_socket *thiz, char *buff, int size);

struct pilot_server_tcp *
pilot_server_tcp_create(struct pilot_application *application, int port)
{
	struct pilot_server_tcp *thiz;
	thiz = malloc(sizeof(*thiz));
	memset(thiz, 0, sizeof(*thiz));
	struct pilot_socket *socket = &thiz->socket;

	LOG_DEBUG("");
	_pilot_socket_init(socket, application, PILOT_SERVER_TCP);
	thiz->port = port;
	thiz->address = NULL;
	thiz->socket.action.disconnect = _pilot_socket_tcp_disconnect;

	_pilot_socket_open((struct pilot_socket *)thiz);
	pilot_connect(socket->connector, dispatch_events, thiz, _pilot_server_tcp_accept);

	_pilot_server_tcp_wait(thiz);
	return thiz;
}

void
pilot_server_tcp_destroy(struct pilot_server_tcp *thiz)
{
	pilot_socket_tcp_destroy((struct pilot_socket_tcp *)thiz);
}

struct pilot_socket_tcp *
_pilot_socket_tcp_create(struct pilot_application *application, char *address, int port)
{
	struct pilot_socket_tcp *thiz;
	thiz = malloc(sizeof(*thiz));
	memset(thiz, 0, sizeof(*thiz));

	_pilot_socket_init(&thiz->socket, application, PILOT_SOCKET_TCP);
	thiz->port = port;
	if (address != NULL)
	{
		thiz->address = malloc(strlen(address)+1);
		strcpy(thiz->address, address);
	}
	thiz->socket.action.disconnect = _pilot_socket_tcp_disconnect;
	thiz->socket.action.read = _pilot_socket_tcp_read;
	thiz->socket.action.write = _pilot_socket_tcp_write;

	return thiz;
}

struct pilot_socket_tcp *
pilot_socket_tcp_create(struct pilot_application *application, char *address, int port)
{
	struct pilot_socket_tcp *thiz;
	struct pilot_socket *socket;
	thiz = _pilot_socket_tcp_create(application, address, port);

	LOG_DEBUG("");
	_pilot_socket_open((struct pilot_socket *)thiz);
	_pilot_socket_tcp_connect(thiz);
	return thiz;
}

void
pilot_socket_tcp_destroy(struct pilot_socket_tcp *thiz)
{
	_pilot_socket_destroy(&thiz->socket);
	if (thiz->address != NULL)
		free(thiz->address);
	LOG_DEBUG("%p", thiz);
	free(thiz);
}

static int
_pilot_socket_tcp_disconnect( struct pilot_socket *thiz)
{
	LOG_DEBUG("%d", thiz->connector->fd);
	pilot_socket_tcp_destroy((struct pilot_socket_tcp *)thiz);
	return -1;
}

static int
_pilot_server_tcp_wait( struct pilot_server_tcp *thiz)
{
	struct pilot_socket *socket = &thiz->socket;
	struct sockaddr_in address;

	LOG_DEBUG("");
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(thiz->port);


	if (thiz->address == NULL)
		address.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		inet_aton(thiz->address, &address.sin_addr);


	return _pilot_socket_wait(socket, (struct sockaddr *)&address, sizeof(address));
}

static int
_pilot_server_tcp_info( struct pilot_server_tcp *thiz, struct pilot_connector *connector)
{
	LOG_DEBUG("%d", connector->fd);
	return 0;
}

static int
_pilot_server_tcp_accept( struct pilot_server_tcp *thiz)
{
	struct sockaddr_in address;
	int add_size = sizeof(address);
	struct pilot_socket *socket = &thiz->socket;
	int fd;
	fd = accept(socket->connector->fd,  (struct sockaddr *)&address, &add_size);
	LOG_DEBUG("accept %d", fd);
	if (fd > 0)
	{
		struct pilot_socket_tcp *newsocket;
		newsocket = _pilot_socket_tcp_create(socket->application, thiz->address, thiz->port);
		_pilot_socket_nonblock(newsocket);
		newsocket->socket.type |= PILOT_SERVER;
		newsocket->socket.connector->fd = fd;
		pilot_emit(thiz, connection, (struct pilot_socket *)newsocket);
		pilot_connect(newsocket->socket.connector, dispatch_events, newsocket, _pilot_socket_dataready);
		if (!newsocket->socket.keepalive)
		{
			pilot_socket_tcp_destroy(newsocket);
		}
		else
		{
			LOG_DEBUG("keepalive");
		}
	}
	else
		return -1;
	return 0;
}

static int
_pilot_socket_tcp_connect( struct pilot_socket_tcp *thiz)
{
	struct sockaddr_in address = {0};

	LOG_DEBUG("");
	address.sin_family = AF_INET;
	address.sin_port = htons(thiz->port);
	if (thiz->address == NULL)
		address.sin_addr.s_addr = htonl(INADDR_ANY);
	else
	{
		int ret;
		ret = inet_aton(thiz->address, &address.sin_addr);
		//ret = inet_pton(address.sin_family, thiz->address, (void *)&address.sin_addr.s_addr);
	}

	return _pilot_socket_connect(&(thiz->socket), (struct sockaddr *)&address, sizeof(address));
}

static int
_pilot_socket_tcp_read(struct pilot_socket *thiz, char *buff, int size)
{
	int ret;
	ret = read(thiz->connector->fd, buff, size);
	LOG_DEBUG("%d", ret);
	if (ret <= 0 && errno != EAGAIN)
	{
		LOG_DEBUG("emit disconnect %d", ret);
		pilot_emit(thiz->connector,disconnect, thiz->connector);
	}
	return ret;
}

static int
_pilot_socket_tcp_write(struct pilot_socket *thiz, char *buff, int size)
{
	return write(thiz->connector->fd, buff, size);
}

#ifdef TEST_SERVER
int
main(int argc, char **argv)
{
	struct pilot_application *application;
	application = pilot_application_create(argc, argv);
	struct pilot_server_tcp *server;
	server = pilot_server_tcp_create(application, 8080);

	pilot_application_run(application);

	pilot_server_tcp_destroy(server);
	pilot_application_destroy(application);
	return 0;
}

#endif
