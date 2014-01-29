#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

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
_pilot_server_local_accept( struct pilot_server_local *thiz);
static int
_pilot_server_local_wait( struct pilot_server_local *thiz);
static int
_pilot_socket_local_disconnect( struct pilot_socket *thiz);
static int
_pilot_socket_local_connect( struct pilot_socket_local *thiz);
static int
_pilot_socket_local_read(struct pilot_socket *thiz, char *buff, int size);
static int
_pilot_socket_local_write(struct pilot_socket *thiz, char *buff, int size);

struct pilot_server_local *
pilot_server_local_create(struct pilot_application *application, char *path)
{
	struct pilot_server_local *thiz;
	thiz = malloc(sizeof(*thiz));
	struct pilot_socket *socket = &thiz->socket;

	LOG_DEBUG("");
	_pilot_socket_init(socket, application, PILOT_SERVER_LOCAL);
	if (path != NULL)
	{
		thiz->address = malloc(strlen(path)+1);
		strcpy(thiz->address, path);
	}
	thiz->socket.action.disconnect = _pilot_socket_local_disconnect;
	thiz->socket.action.read = _pilot_socket_local_read;
	thiz->socket.action.write = _pilot_socket_local_write;


	_pilot_socket_open((struct pilot_socket *)thiz);
	pilot_connect(socket->connector, dispatch_events, thiz, _pilot_server_local_accept);

	_pilot_server_local_wait(thiz);
	return thiz;
}

void
pilot_server_local_destroy(struct pilot_server_local *thiz)
{
	pilot_socket_local_destroy((struct pilot_socket_local *)thiz);
}

struct pilot_socket_local *
_pilot_socket_local_create(struct pilot_application *application, char *path)
{
	struct pilot_socket_local *thiz;
	thiz = malloc(sizeof(*thiz));

	_pilot_socket_init(&thiz->socket, application, PILOT_SOCKET_LOCAL);
	if (path != NULL)
	{
		thiz->address = malloc(strlen(path)+1);
		strcpy(thiz->address, path);
	}
	thiz->socket.action.disconnect = _pilot_socket_local_disconnect;
	thiz->socket.action.read = _pilot_socket_local_read;
	thiz->socket.action.write = _pilot_socket_local_write;

	return thiz;
}

struct pilot_socket_local *
pilot_socket_local_create(struct pilot_application *application, char *path)
{
	struct pilot_socket_local *thiz;
	struct pilot_socket *socket;
	thiz = _pilot_socket_local_create(application, path);

	LOG_DEBUG("");
	_pilot_socket_open((struct pilot_socket *)thiz);
	_pilot_socket_local_connect(thiz);
	pilot_connect(thiz->socket.connector, disconnect, thiz, _pilot_socket_disconnect);
	return thiz;
}

void
pilot_socket_local_destroy(struct pilot_socket_local *thiz)
{
	_pilot_socket_destroy(&thiz->socket);
	if (thiz->address != NULL)
	{
		unlink(thiz->address);
		free(thiz->address);
	}
	free(thiz);
}

static int
_pilot_socket_local_disconnect( struct pilot_socket *thiz)
{
	LOG_DEBUG("%d", thiz->connector->fd);
	pilot_socket_local_destroy((struct pilot_socket_local *)thiz);
	return -1;
}

static int
_pilot_server_local_wait( struct pilot_server_local *thiz)
{
	struct pilot_socket *socket = &thiz->socket;
	struct sockaddr_un address;

	LOG_DEBUG("");
	memset(&address, 0, sizeof(address));
	address.sun_family = AF_LOCAL;

	strncpy(address.sun_path, thiz->address,
                   sizeof(address.sun_path) - 1);

	return _pilot_socket_wait(socket, (struct sockaddr *)&address, sizeof(address));
}

static int
_pilot_server_local_accept( struct pilot_server_local *thiz)
{
	struct sockaddr_un address;
	int add_size = sizeof(address);
	struct pilot_socket *socket = &thiz->socket;
	int fd;
	fd = accept(socket->connector->fd,  (struct sockaddr *)&address, &add_size);
	LOG_DEBUG("accept %d", fd);
	if (fd > 0)
	{
		struct pilot_socket_local *newsocket;
		newsocket = _pilot_socket_local_create(socket->application, thiz->address);
		newsocket->socket.type |= PILOT_SERVER;
		newsocket->socket.connector->fd = fd;
		pilot_connect(newsocket->socket.connector, dispatch_events, newsocket, _pilot_socket_dataready);
		pilot_emit(thiz, connection, (struct pilot_socket *)newsocket);
		if (!newsocket->socket.keepalive)
		{
			pilot_socket_local_destroy(newsocket);
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
_pilot_socket_local_connect( struct pilot_socket_local *thiz)
{
	struct sockaddr_un address;

	LOG_DEBUG("");
	memset(&address, 0, sizeof(address));
	address.sun_family = AF_LOCAL;

	strncpy(address.sun_path, thiz->address,
                   sizeof(address.sun_path) - 1);

	return _pilot_socket_connect(&(thiz->socket), (struct sockaddr *)&address, sizeof(address));
}

static int
_pilot_socket_local_read(struct pilot_socket *thiz, char *buff, int size)
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
_pilot_socket_local_write(struct pilot_socket *thiz, char *buff, int size)
{
	return write(thiz->connector->fd, buff, size);
}

#ifdef TEST_SERVER
int
main(int argc, char **argv)
{
	struct pilot_application *application;
	application = pilot_application_create(argc, argv);
	struct pilot_server_local *server;
	server = pilot_server_local_create(application, 8080);

	pilot_application_run(application);

	pilot_server_local_destroy(server);
	pilot_application_destroy(application);
	return 0;
}

#endif
