src = $(wildcard *.c)
obj = $(src:%.c=%.o)

CC = gcc
CFLAGS = -lpthread -g

server:server.o
	$(CC) $^ $(CFLAGS) -o $@
client:client.o
	$(CC) $^ $(CFLAGS) -o $@

%.o:%.c
	$(CC) $^ $(CFLAGS) -c -o $@

.PHONY:all
all:server client

.PHONY:clean
clean:
	rm -fr $(obj) server client
