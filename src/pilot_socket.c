#include <sys/types.h>
#include <sys/socket.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include <pilot_atk.h>
#include <pilot_ntk.h>
#include "pilot_socket.h"

struct pilot_socket *
pilot_socket_create(struct pilot_application *application, int type)
{
	struct pilot_socket *thiz;
	thiz = malloc(sizeof(*thiz));
	memset(thiz, 0, sizeof(*thiz));
	LOG_DEBUG("%p", thiz);

	_pilot_socket_init(thiz, application, type);
	return thiz;
}

struct pilot_socket *
pilot_socket_dup(struct pilot_socket *socket)
{
	struct pilot_socket *thiz;
	thiz = malloc(sizeof(*thiz));
	memset(thiz, 0, sizeof(*thiz));
	LOG_DEBUG("%p", thiz);

	_pilot_socket_init(thiz, socket->application, socket->type);
	thiz->action.disconnect = pilot_socket_destroy;
	thiz->action.read = socket->action.read;
	thiz->action.write = socket->action.write;

	thiz->connector->fd = dup(socket->connector->fd);
	_pilot_socket_nonblock(thiz);
	return thiz;
}

void
_pilot_socket_destroy(struct pilot_socket *thiz)
{		
	LOG_DEBUG("");
	pilot_connector_destroy(thiz->connector);
	close(thiz->connector->fd);
}

void
pilot_socket_destroy(struct pilot_socket *thiz)
{
	_pilot_socket_destroy(thiz);
	free(thiz);
}

int
_pilot_socket_init(struct pilot_socket *thiz, struct pilot_application *application, int type)
{
	thiz->application = application;
	thiz->type = type;
	if ((type | PILOT_SERVER) == PILOT_SERVER_TCP)
	{
		thiz->layer = PF_INET;
		thiz->schema = SOCK_STREAM;
		thiz->protocol = 0;
	}
	if ((type | PILOT_SERVER) == PILOT_SERVER_UDP)
	{
		thiz->layer = PF_INET;
		thiz->schema = SOCK_DGRAM;
		thiz->protocol = 0;
	}
	if ((type | PILOT_SERVER) == PILOT_SERVER_LOCAL)
	{
		thiz->layer = AF_LOCAL;
		thiz->schema = SOCK_STREAM;
		thiz->protocol = 0;
	}
	thiz->connector = pilot_connector_create(application);
	pilot_connect(thiz->connector, disconnect, thiz, _pilot_socket_disconnect);

	return 0;
}

int
_pilot_socket_open( struct pilot_socket *thiz)
{
	int fd;
	int yes = 1;
	int fcnt;

	LOG_DEBUG("socket");
	if ((fd = socket(thiz->layer, thiz->schema, thiz->protocol)) < 0)
	{
		LOG_DEBUG("error socket: %s", strerror(errno));
		return -errno;
	}
	LOG_DEBUG("reuse");
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
	{
		LOG_DEBUG("error reuse opt: %s", strerror(errno));
		close(fd);
		return -errno;
	}
	thiz->connector->fd = fd;
	return fd;
}

int
_pilot_socket_wait( struct pilot_socket *thiz, struct sockaddr *address, int addsize)
{
	int fd = thiz->connector->fd;
	int yes = 1;

	LOG_DEBUG("bind");
	if ( bind(fd,address, addsize) < 0)
	{
		LOG_DEBUG("error bind: %s", strerror(errno));
		close(fd);
		thiz->connector->fd = -1;
		return -errno;
	}
	LOG_DEBUG("listen");
	if ((thiz->schema == SOCK_STREAM) && (listen(fd, 10) < 0))
	{
		LOG_DEBUG("error listen: %s", strerror(errno));
		close(fd);
		thiz->connector->fd = -1;
		return -errno;
	}
	return _pilot_socket_nonblock(thiz);
}

int
_pilot_socket_dataready( struct pilot_socket *thiz)
{
	int ret;
	int value = 0;
	ret = ioctl(thiz->connector->fd, FIONREAD, &value);
	LOG_DEBUG("%d bytes", value);
	pilot_emit(thiz, dataready, thiz, value);
	return ret;
}

int
_pilot_socket_connect( struct pilot_socket *thiz, struct sockaddr *address, int addsize)
{
	int fd = thiz->connector->fd;
	int ret = 0;

	if (fd < 0)
	{
		return -1;
	}
	LOG_DEBUG("connect");
	if (connect(fd, address, addsize) < 0)
	{
		LOG_DEBUG("error connect: %s", strerror(errno));
		close(fd);
		thiz->connector->fd = -1;
		return -errno;
	}
	ret = _pilot_socket_nonblock(thiz);
	pilot_connect(thiz->connector, dispatch_events, thiz, _pilot_socket_dataready);
	return ret;
}

int
_pilot_socket_nonblock(struct pilot_socket *thiz)
{
	int fd = thiz->connector->fd;
	int yes = 1;

	if ((yes = fcntl(fd,F_GETFL,0)) < 0)
	{
		close(fd);
		thiz->connector->fd = -1;
		return -errno;
	}
	else if ( fcntl(fd,F_SETFL,yes | O_NONBLOCK) < 0)
	{
		LOG_DEBUG("error nonblock: %s", strerror(errno));
		close(fd);
		thiz->connector->fd = -1;
		return -errno;
	}
	return 0;
}

int pilot_socket_run(struct pilot_socket *thiz)
{
	int ret = 0;
	do
	{
		ret = pilot_connector_wait(thiz->connector);
	} while (ret >= 0);
	return ret;
}

int
pilot_socket_read(struct pilot_socket *thiz, char *buff, int size)
{
	int ret = 0;
	if (thiz->action.read)
	{
		ret = thiz->action.read(thiz, buff, size);
	}
	else
	{
		LOG_DEBUG("no read function availlable");
		ret = read(thiz->connector->fd, buff, size);
	}
	if (ret < 0 && errno == EAGAIN )
		ret = 0;
	return ret;
}

int
pilot_socket_write(struct pilot_socket *thiz, char *buff, int size)
{
	if (thiz->action.write)
	{
		return thiz->action.write(thiz, buff, size);
	}
	return 0;
}

int
_pilot_socket_disconnect( struct pilot_socket *thiz)
{
	if (thiz->action.disconnect)
		return thiz->action.disconnect(thiz);
	return 0;
}

