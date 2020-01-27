  
CC=gcc
CFLAGS=-O3 -Wall
LIBS=-pthread

all: chat_server.out chat_client.out
chat_server.out: chat_server.o
	$(CC) $(CFLAGS) $(LIBS) -o chat_server.out chat_server.o
chat_client.out: chat_client.o string.o proto.o
	$(CC) $(CFLAGS) $(LIBS) -o chat_client.out chat_client.o string.o proto.o
chat_server.o: chat_server.c
	$(CC) $(CFLAGS) -c chat_server.c
string.o: string.c
	$(CC) $(CFLAGS) -c string.c
chat_client.o: chat_client.c
	$(CC) $(CFLAGS) -c chat_client.c
proto.o: proto.c
	$(CC) $(CFLAGS) -c proto.c


.PHONY: clean
clean:
	rm -f *.o *.out