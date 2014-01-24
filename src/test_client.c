#include <stdlib.h>

#include <pilot_atk.h>
#include <pilot_ntk.h>

struct pilot_application *g_application;

int
testconnection(void *unused, struct pilot_socket_tcp *socket)
{
	printf("coucou");
}
int
main(int argc, char **argv)
{
	int ret = 0;
	struct pilot_socket_tcp *client;
	g_application = pilot_application_create(argc, argv);

#ifdef TCP
	client = pilot_socket_tcp_create(g_application, "127.0.0.1", 8080);
#else
	client = pilot_socket_local_create(g_application, "/tmp/pilot");
#endif
	LOG_DEBUG("");

	pilot_socket_write((struct pilot_socket*)client, "coucou\n", 8);
	ret = pilot_application_run(g_application);

	pilot_application_destroy(g_application);
	return ret;
}
