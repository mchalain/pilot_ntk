bin-$(CONFIG_PILOT_TEST)+=test_server test_client

TYPE=TCP
test_server_CFLAGS=$(CONFIG_PILOT_CFLAGS) -D$(TYPE)
test_server_LDFLAGS=$(CONFIG_PILOT_LDFLAGS)
test_server_LIBRARY=$(CONFIG_PILOT_LIBRARY)

test_client_CFLAGS=$(CONFIG_PILOT_CFLAGS) -D$(TYPE)
test_client_LDFLAGS=$(CONFIG_PILOT_LDFLAGS)
test_client_LIBRARY=$(CONFIG_PILOT_LIBRARY)
