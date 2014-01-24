#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <pilot_atk.h>
#include <pilot_ntk.h>

static int
testlog(struct pilot_service *service)
{
	char buff[256];
	int ret;
	buff[0] = 0;
	ret = pilot_socket_read(service->socket, buff, sizeof(buff));
	LOG_DEBUG("%d %s", ret, buff);
	return ret;
}
int
testconnection(void *unused, struct pilot_socket *socket)
{
	struct pilot_service *service;
	/// three solutions :
	///  first : I want to recieve a request send an answer and close
	///    I use this socket to read and write and return 0;
	/*
	 * char buff[256];
	 * pilot_socket_read(socket, buff, sizeof(buff));
	 * LOG_DEBUG("%d %s", ret, buff);
	 * pilot_socket_write(socket, "Hello", 7);
	 */
	///  second : I want my own socket
	///    I dup the given one, and I use here or inside a fork
	///    I destroy the socket at the end of the service
	socket = pilot_socket_dup(socket);
	///  third : I keep the socket alive
	///    I set the keepalive flag to the socket,
	///    I use here, create a service and destroy it at the end.
	/*
	 * 	socket->keepalive = 1;
	 */
	service = pilot_service_create(socket);
	service->action.recieve_server=testlog;
	printf("coucou");
	return 0;
}
int
main(int argc, char **argv)
{
	int ret = 0;
	struct pilot_server_tcp *server;
	struct pilot_application *g_application = pilot_application_create(argc, argv);

#ifdef TCP
	server = pilot_server_tcp_create(g_application, 8080);
#else
	server = pilot_server_local_create(g_application, "/tmp/pilot");
#endif
	pilot_connect(server, connection, NULL, testconnection);

	LOG_DEBUG("");

	ret = pilot_application_run(g_application);

	pilot_application_destroy(g_application);
	return ret;
}
