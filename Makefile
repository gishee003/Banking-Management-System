CC = gcc
CFLAGS = -Wall -Iinclude

all: server client initdb

server: src/server.c src/database.c src/session.c
	$(CC) $(CFLAGS) $^ -o server

client: src/client.c src/customer_menu.c
	$(CC) $(CFLAGS) $^ -o client

initdb: tools/initdb.c
	$(CC) $(CFLAGS) $^ -o initdb

clean:
	rm -f server client initdb session

