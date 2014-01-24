obj=obj
src=src
srctree=.

bin-y=
lib-y=

bin-ext=
slib-ext=a
dlib-ext=so

include ./config

CFLAGS=-g -DDEBUG
STATIC=

include $(src)/pilot_ntk.mk

include ./scripts.mk

clean:
	$(RM) $(target-objs)
distclean: clean
	$(RM) $(lib-dynamic-target) $(lib-static-target)

