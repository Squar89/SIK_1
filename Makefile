CC=gcc
PARAMS=-Wall -Wextra -O2 -std=c11

all: server

server: server.c
	$(CC) $(PARAMS) -o server server.c

clean:
	rm -rf server