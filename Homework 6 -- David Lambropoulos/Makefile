cc = gcc
CFLAGS = -g
LDLIBS = -lpthread

TARGETS = server client

default: all

all : $(TARGETS)

server: server.o
	$(CC) -o $@ server.o $(LDLIBS)

client: client.o
	$(CC) -o $@ client.o $(LDLIBS)

server.o: server.c server.h
	$(CC) -c $(CFLAGS) server.c

client.o: client.c client.h
	$(CC) -c $(CFLAGS) client.c	

clean:
	rm -f *~ *.o server client 
