bin-$(CONFIG_PILOT_TEST)+=test_server test_client

test_server_CFLAGS=$(CONFIG_PILOT_CFLAGS)
test_server_LDFLAGS=$(CONFIG_PILOT_LDFLAGS)
test_server_LIBRARY=$(CONFIG_PILOT_LIBRARY)

test_client_CFLAGS=$(CONFIG_PILOT_CFLAGS)
test_client_LDFLAGS=$(CONFIG_PILOT_LDFLAGS)
test_client_LIBRARY=$(CONFIG_PILOT_LIBRARY)