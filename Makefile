CC = gcc
CFLAGS = -Wall -Iinclude

all: server client initdb

server: src/server.c src/database.c src/session.c src/customer_ops.c
	$(CC) $(CFLAGS) $^ -o server

client: src/client.c 
	$(CC) $(CFLAGS) $^ -o client

initdb: tools/initdb.c
	$(CC) $(CFLAGS) $^ -o initdb

clean:
	rm -f server client initdb session

