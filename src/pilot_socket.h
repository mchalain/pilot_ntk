#ifndef __PILOT_SOCKAT_PRIVATE_H__
#define __PILOT_SOCKAT_PRIVATE_H__

int
_pilot_socket_disconnect( struct pilot_socket *thiz);
int
_pilot_socket_wait( struct pilot_socket *thiz, struct sockaddr *address, int addsize);
int
_pilot_socket_connect( struct pilot_socket *thiz, struct sockaddr *address, int addsize);
int
_pilot_socket_open( struct pilot_socket *thiz);
int
_pilot_socket_init(struct pilot_socket *thiz, struct pilot_application *application, int type);
void
_pilot_socket_destroy(struct pilot_socket *thiz);
int
_pilot_socket_dataready( struct pilot_socket *thiz);

#endif
