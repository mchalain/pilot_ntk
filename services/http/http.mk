slib-$(CONFIG_PILOT_SERVICES_BAD)+=httpparser
httpparser_SOURCES= \
	../services/http/tools.c \
	../services/http/httprequest.c \
	../services/http/httpresponse.c  \
	../services/http/content.c
httpparser-objs:=$(httpparser_SOURCES:%.c=%.o)
$(foreach s, $(httpparser-objs), $(eval $(s:%.o=%)_CFLAGS+=$(httpparser_CFLAGS)) )

lib-$(CONFIG_PILOT_SERVICES_BAD)+=service_http
service_http_SOURCES= \
	../services/http/pilot_servicehttp.c \
	 libhttpparser.a
service_http-objs=$(service_http_SOURCES:%.c=%.o)
service_http_CFLAGS=$(CONFIG_PILOT_CFLAGS)
service_http_LDFLAGS=$(CONFIG_PILOT_LDFLAGS)
service_http_LIBRARY=$(CONFIG_PILOT_LIBRARY)
$(foreach s, $(service_http-objs), $(eval $(s:%.o=%)_CFLAGS+=$(service_http_CFLAGS)) )
