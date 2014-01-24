ifeq ($(CONFIG_PILOT_PTHREAD),y)
CONFIG_PILOT_CFLAGS+=-DPTHREAD
CONFIG_PILOT_LDFLAGS+=-pthread
CONFIG_PILOT_LIBRARY+=pthread
endif

lib-$(CONFIG_PILOT_NTK)+=pilot_ntk
pilot_ntk_SOURCES=pilot_socket.c pilot_service.c
pilot_ntk_$(CONFIG_PILOT_NTK_TCP)+=pilot_socket_tcp.c
pilot_ntk_$(CONFIG_PILOT_NTK_UDP)+=pilot_socket_udp.c
pilot_ntk_$(CONFIG_PILOT_NTK_LOCAL)+=pilot_socket_local.c
pilot_ntk_SOURCES+=$(pilot_ntk_y)
pilot_ntk-objs=$(pilot_ntk_SOURCES:%.c=%.o)

pilot_ntk_CFLAGS=$(CONFIG_PILOT_CFLAGS)
pilot_ntk_LDFLAGS=$(CONFIG_PILOT_LDFLAGS)
pilot_ntk_LIBRARY=

$(foreach s, $(pilot_ntk-objs), $(eval $(s:%.o=%)_CFLAGS+=$(pilot_ntk_CFLAGS)) )

include $(src)/test.mk
