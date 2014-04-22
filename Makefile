default: clean debug

debug: DEBUG=-g -D DEBUG

release: DEBUG=

debug: all

release: all

LINK=-lpthread

all: main.o util/server.o util/client_list.o
	gcc -o server main.o util/server.o util/client_list.o $(DEBUG) $(LINK)

main.o: main.c
	gcc -c main.c -o main.o $(DEBUG)

util/server.o:
	gcc -c util/server.c -o util/server.o $(DEBUG)

util/client_list.o:
	gcc -c util/client_list.c -o util/client_list.o $(DEBUG)

clean:
	rm -rf *.o util/*.o server

