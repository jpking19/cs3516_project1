CC = gcc
CFLAGS = -g -Wall

all: http_client http_server

http_client: http_client.o
	$(CC) $(CFLAGS) -o http_client http_client.o

http_server: http_server.o
	$(CC) $(CFLAGS) -o http_server http_server.o -lpthread

http_client.o: http_client.c
	$(CC) $(CFLAGS) -c http_client.c

http_server.o: http_server.c
	$(CC) $(CFLAGS) -c http_server.c

clean:
	rm -rf http_client http_server http_client.o http_server.o
