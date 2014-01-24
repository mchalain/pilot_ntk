/**
 * Project: libpilot_ntk
 * Author: Marc Chalain (Copyright (C) 2013)
 * file: pilot_server.c
 * description: this object manages the main socket and the connections from the clients.
 *
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <errno.h>

#include <pilot_server.h>
#include <pilot_atk.h>
#include "connector.h"
#include "service_private.h"

#define MAX(x,y)  (x)>(y)?(x):(y)
#define MIN(x,y) (x)<(y)?(x):(y)

#define MAX_CONNECTOR 10

//log("%s: Leroy was here connectors\n", __FUNCTION__);
/**
 * structures
 **/

struct pilot_server
{
	struct pilot_application *application;
	struct pilot_connector *connector;
	struct pilot_net_service *service;
	_pilot_list(pilot_clientadapter, clients);

};


/**
 * internal API
 **/
static int
_pilot_server_prepare_wait(struct pilot_server *thiz);

/**
 * functions definition
 **/
struct pilot_server *
pilot_server_create(struct pilot_application *application, struct pilot_net_service *service)
{
	struct pilot_server *thiz =malloc(sizeof(*thiz));
	memset(thiz, 0, sizeof(*thiz));
	thiz->connector =
		pilot_connector_create(thiz);
	thiz->service = service;
	thiz->connector->action.prepare_wait = _pilot_server_prepare_wait;
	thiz->connector->action.dispatch_events = _pilot_server_dispatch_events;

	return thiz;
}

int
pilot_server_open(struct pilot_server *thiz, struct pilot_socket *socket)
{

	thiz->connector->fd = pilot_socket_open(socket);
	pilot_application_addconnector(thiz->application, thiz->connector);
	return 0;
}

static int
_pilot_server_destroy_clientadapter(struct pilot_server *thiz, struct pilot_clientadapter *adapter)
{
	pilot_clientadapter_destroy(adapter);
}

void
pilot_server_destroy(struct pilot_server *thiz)
{
	int i;
	pilot_list_foreach(thiz->clients, _pilot_server_destroy_clientadapter, thiz);
	pilot_list_destroy(thiz->clients);
	free(thiz);
}

static int
_pilot_server_prepare_wait(struct pilot_server *thiz)
{
	int ret = 0;
	return ret;
}

static int
_pilot_server_dispatch_events(struct pilot_server *thiz)
{
	int fd;
	fd = pilot_socket_accept(thiz->socket, thiz->connector->fd);

	client = pilot_clientadapter_create(fd, thiz->application, thiz->service);
	if (client)
		pilot_list_append(thiz->clients, client);

	return 0;
}

int
server_main( struct pilot_server *thiz)
{
	int ret;
	fd_set rfds;

	log("wait connection\n");
	this->m_run = 1;
	do
	{
		int maxfd = server_getmaxfd(thiz, &rfds);
		log("maxfd %d\n", maxfd);
		if (maxfd == 0)
		{
			server_stop(thiz);
			break;
		}
		ret = select(maxfd + 1, &rfds, NULL, NULL, NULL);
		if (ret > 0 )
		{
			int i;

			for (i = 0; i < thiz->m_nbconnectors; i++)
			{
				Connector *connector = thiz->m_connector[i];
				if (FD_ISSET(connector_getsocket(connector), &rfds) > 0)
				{
					ClientAdapter *client = connector_connected(connector);
					if (client)
					{
						server_addclient(thiz, client);
					}
					else
					{
						server_closeconnector(thiz, connector);
						// ugly patch to end if trouble on the UDP socket
						if (i == 0)
							server_stop(this);
					}
					break;
				}
			}

			for (i = 0; i < thiz->m_nbclients; i++)
			{
				ClientAdapter *client = thiz->m_client[i];
				if (FD_ISSET(clientadapter_getsocket(client), &rfds))
				{
					ret = service_main(clientadapter_service(client));
					if (ret < 0)
					{
						server_closeclient(thiz, client);
					}
					break;
				}
			}
		}
		else if (ret < 0)
		{
			error("error on socket :(%d) %s\n", errno, strerror(errno));
			if (errno != EINTR)
				server_stop(thiz);
		}
	}
	while(thiz->m_run);

	return 0;
}

void
server_stop(struct pilot_server *thiz)
{
	int i;
	thiz->m_run = 0;
	for (i = 0; i < this->m_nbclients; i++)
	{
		ClientAdapter *client = thiz->m_client[i];
		int fd = clientadapter_getsocket(client);
		shutdown(fd, SHUT_RDWR);
	}
	for (i = 0; i < thiz->m_nbconnectors; i++)
	{
		Connector *connector = thiz->m_connector[i];
		int fd = connector_getsocket(connector);
		shutdown(fd, SHUT_RDWR);
	}
}



int
server_addconnector(struct pilot_server *thiz, Connector *connector)
{
	int ret = -1, i;
	int serviceid = connector_getserviceid(connector);

	for (i = 0; i < MAX_CONNECTOR; i++)
	{
		Connector *cur = thiz->m_connector[i];
		if (cur == NULL)
		{
			thiz->m_connector[i] = connector;
			thiz->m_nbconnectors++;
			if ((ret = connector_createsrvsocket(connector)) < 0)
			{
				error("error : %s\n", strerror(errno));
				return ret;
			}
			ret = connector_waitclient(connector);
			break;
		}
		else if (serviceid == connector_getserviceid(cur))
		{
			//service is already running
			log("service is already running\n");
			break;
		}
	}
	return ret;
}

void
server_closeconnector(struct pilot_server *thiz, Connector *connector)
{
	int j = 0;

	// search the client to close in the list
	while (thiz->m_connector[j] != connector && thiz->m_connector[j] != NULL) j++;
	if (thiz->m_connector[j] == NULL)
	{
		fprintf(stderr, "connector not found on port %d\n", connector_getport(connector));
		return;
	}
	fprintf(stderr, " close connector on port %d\n", connector_getport(connector));
	// close the client
	connector_destroy(thiz->m_connector[j]);
	j++;
	// sort the clients' list
	while (thiz->m_connector[j] != NULL)
	{
		thiz->m_connector[j - 1] = thiz->m_connector[j];
		j++;
	}
	thiz->m_nbconnectors--;
	thiz->m_connector[thiz->m_nbconnectors] = NULL;
	if (thiz->m_nbconnectors == 0)
		server_stop(this);
}

int
server_addclient(struct pilot_server *thiz, ClientAdapter *client)
{
	int ret = -1;

	if (thiz->m_nbclients < MAX_CLIENT)
	{
		thiz->m_client[thiz->m_nbclients] = client;
		thiz->m_nbclients++;
		ret = 0;
	}
	return ret;
}

void
server_closeclient(struct pilot_server *thiz, ClientAdapter *client)
{
	int j = 0;

	// search the client to close in the list
	while (thiz->m_client[j] != client && thiz->m_client[j] != NULL) j++;
	// close the client
	clientadapter_destroy(thiz->m_client[j]);
	j++;
	// sort the clients' list
	while (thiz->m_client[j] != NULL)
	{
		thiz->m_client[j - 1] = thiz->m_client[j];
		j++;
	}
	thiz->m_nbclients--;
	thiz->m_client[thiz->m_nbclients] = NULL;
}

int
server_getmaxfd(struct pilot_server *thiz, fd_set *fds)
{
	int maxfd = 0;
	int j = 0;
	int fd;

	FD_ZERO(fds);
	while (thiz->m_connector[j] != NULL)
	{
		Connector *connector = thiz->m_connector[j];
		fd = connector_getsocket(connector);
		if (!FD_ISSET(fd, fds))
		{
			FD_SET(fd, fds);
			maxfd = MAX(maxfd, fd);
		}
		j++;
	}
	j = 0;
	while (thiz->m_client[j] != NULL)
	{
		ClientAdapter *client = thiz->m_client[j];
		fd = clientadapter_getsocket(client);
		if (!FD_ISSET(fd, fds))
		{
			FD_SET(fd,fds);
			maxfd = MAX(maxfd, fd);
		}
		j++;
	}
	return maxfd;
}

/**
 * API
 **/
