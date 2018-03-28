src = $(wildcard *.c)
obj = $(src:%.c=%.o)
CC = gcc

server:server.o
	$(CC) $^ -o $@
client:client.o
	$(CC) $^ -o $@

%.o:%.c
	$(CC) -c $^ -o $@

.PHONY:all
all:server client

.PHONY:clean
clean:
	rm -fr $(obj) server client
