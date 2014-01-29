#ifndef __PILOT_NTK_H__
#define __PILOT_NTK_H__

#include <pilot_atk.h>

#define PILOT_SERVER		0x10

#define PILOT_SOCKET_TCP	0x01
#define PILOT_SERVER_TCP	(PILOT_SERVER | PILOT_SOCKET_TCP)
#define PILOT_SOCKET_UDP	0x02
#define PILOT_SERVER_UDP	(PILOT_SERVER | PILOT_SOCKET_UDP)
#define PILOT_SOCKET_LOCAL	0x04
#define PILOT_SERVER_LOCAL	(PILOT_SERVER | PILOT_SOCKET_LOCAL)

struct pilot_socket
{
	struct pilot_application *application;
	struct pilot_connector *connector;
	struct {
		int (*read)(struct pilot_socket *, char *, int);
		int (*write)(struct pilot_socket *, char *, int);
		int (*disconnect)(struct pilot_socket *);
	} action;
	_pilot_signal(pilot_socket, dataready, struct pilot_socket *socket, int len);
	char type;
	char keepalive:1;
	int layer;
	int schema;
	int protocol;
};

struct pilot_socket *
pilot_socket_create(struct pilot_application *application, int type);
struct pilot_socket *
pilot_socket_dup(struct pilot_socket *socket);
void
pilot_socket_destroy(struct pilot_socket *thiz);
int
pilot_socket_read(struct pilot_socket *thiz, char *buff, int size);
int
pilot_socket_write(struct pilot_socket *thiz, char *buff, int size);

/************************************************************************
 ** IP  v4 socket and server
 ************************************************************************/
struct pilot_socket_inet
{
	struct pilot_socket socket;
	int port;
	char *address;
};

struct pilot_server_inet
{
	struct pilot_socket socket;
	int port;
	char *address;
	_pilot_signal(pilot_server_inet, connection, struct pilot_socket *socket);
};

/************************************************************************
 ** IP  v4 socket and server TCP
 ************************************************************************/
#define pilot_socket_tcp pilot_socket_inet
#define pilot_server_tcp pilot_server_inet
struct pilot_server_tcp *
pilot_server_tcp_create(struct pilot_application *application, int port);
void
pilot_server_tcp_destroy(struct pilot_server_tcp *);

struct pilot_socket_tcp *
pilot_socket_tcp_create(struct pilot_application *application, char *address, int port);
void
pilot_socket_tcp_destroy(struct pilot_socket_tcp *);

/************************************************************************
 ** IP  v4 socket and server UDP
 ************************************************************************/
#define BROADCAST "BROADCAST"
#define MULTICAST "MULTICAST"

#define pilot_socket_udp pilot_socket_inet
#define pilot_server_udp pilot_server_inet
struct pilot_server_udp *
pilot_server_udp_create(struct pilot_application *application, int port);
void
pilot_server_udp_destroy(struct pilot_server_udp *);

struct pilot_socket_udp *
pilot_socket_udp_create(struct pilot_application *application, char *address, int port);
void
pilot_socket_udp_destroy(struct pilot_socket_udp *);

/************************************************************************
 ** UNIX Local socket and server
 ************************************************************************/
struct pilot_socket_local
{
	struct pilot_socket socket;
	int port;
	char *address;
};

struct pilot_server_local
{
	struct pilot_socket socket;
	int port;
	char *address;
	_pilot_signal(pilot_server_local, connection, struct pilot_socket *socket);
};

struct pilot_server_local *
pilot_server_local_create(struct pilot_application *application, char *path);
void
pilot_server_local_destroy(struct pilot_server_local *);

struct pilot_socket_local *
pilot_socket_local_create(struct pilot_application *application, char *path);
void
pilot_socket_local_destroy(struct pilot_socket_local *);

/************************************************************************
 ** the service tool
 ** the service define the treatment  of the data from a socket
 ************************************************************************/
struct pilot_service
{
	struct pilot_socket *socket;
	struct
	{
		int (*recieve_server)(struct pilot_service *thiz);
		int (*recieve_client)(struct pilot_service *thiz);
	} action;
};

struct pilot_service *
pilot_service_create(struct pilot_socket *socket);
void
pilot_service_destroy(struct pilot_service *thiz);

#endif
