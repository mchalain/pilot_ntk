#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <pilot_atk.h>
#include <pilot_ntk.h>

static int
_pilot_service_receive_server(struct pilot_service *thiz, struct pilot_connector *connector);
static int
_pilot_service_receive_client(struct pilot_service *thiz, struct pilot_connector *connector);
static int
_pilot_service_disconnect(struct pilot_service *thiz);

struct pilot_service *
pilot_service_create(struct pilot_socket *socket)
{
	struct pilot_service *thiz;
	thiz = malloc(sizeof(*thiz));

	memset(thiz, 0, sizeof(*thiz));
	thiz->socket = socket;
	if (thiz->socket->type & PILOT_SERVER)
		pilot_connect(thiz->socket->connector, dispatch_events, thiz, _pilot_service_receive_server);
	if ((~thiz->socket->type) & PILOT_SERVER)
		pilot_connect(thiz->socket->connector, dispatch_events, thiz, _pilot_service_receive_client);
	pilot_connect(thiz->socket->connector, disconnect, thiz, _pilot_service_disconnect);
	return thiz;
}

static int
_pilot_service_disconnect(struct pilot_service *thiz)
{
	pilot_service_destroy(thiz);
	return -1;
}

void
pilot_service_destroy(struct pilot_service *thiz)
{
	pilot_disconnect(thiz->socket->connector, dispatch_events,thiz);
	pilot_disconnect(thiz->socket->connector, disconnect,thiz);
	if (thiz->action.destroy)
		thiz->action.destroy(thiz);
	free(thiz);
}

static int
_pilot_service_receive_server(struct pilot_service *thiz, struct pilot_connector *connector)
{
	int ret = 0;
	if (thiz->action.receive_server)
	{
		ret = thiz->action.receive_server(thiz);
	}
	else
	{
		char buff[2];
		do
		{
			ret = thiz->socket->action.read(thiz->socket, buff, sizeof(buff));
		} while ( ret > 0);
		LOG_DEBUG("%s", buff);
	}
	connector->distribut = ret;
	return ret;
}

static int
_pilot_service_receive_client(struct pilot_service *thiz, struct pilot_connector *connector)
{
	int ret = 0;
	char buff[2];
	if (thiz->action.receive_client)
		ret = thiz->action.receive_client(thiz);
	else
	{
		while (thiz->socket->action.read(thiz->socket, buff, sizeof(buff)) > 0);
		LOG_DEBUG("%s", buff);
	}
	return ret;
}
