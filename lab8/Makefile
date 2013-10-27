CC=g++

CFLAGS=-Wall -std=c++0x


all: 
	$(CC) $(CFLAGS) client.c -o client
	$(CC) $(CFLAGS) server.cpp -o server -lpthread

clean:
	rm -f client
	rm -f server
