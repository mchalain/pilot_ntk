#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <pilot_atk.h>
#include <pilot_ntk.h>

int
clientconnection(void *unused, struct pilot_socket *socket)
{
	socket = pilot_socket_dup(socket);
	if (fork() == 0)
	{
		struct pilot_service *service;
		LOG_DEBUG("fork");
		service = pilot_servicehttp_create(socket);
		pilot_socket_run(socket);
		pilot_service_destroy(service);
		pilot_socket_destroy(socket);
		exit(0);
	}
	pilot_socket_destroy(socket);
	
	return 0;
}

int
main(int argc, char **argv)
{
	int ret = 0;
	struct pilot_server_tcp *server;
	struct pilot_application *g_application = pilot_application_create(argc, argv);

	server = pilot_server_tcp_create(g_application, 8080);
	pilot_connect(server, connection, NULL, clientconnection);

	LOG_DEBUG("");

	ret = pilot_application_run(g_application);

	pilot_application_destroy(g_application);
	return ret;
}
