CC      = gcc
CFLAGS  = -Wall -Wextra -g
LDFLAGS = -lssl -lcrypto

SRCS_SERVER = authserver.c utils.c
SRCS_CLIENT = authclient.c utils.c

all: authserver authclient

authserver: $(SRCS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

authclient: $(SRCS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f authserver authclient

.PHONY: all clean
