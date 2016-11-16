SRC := $(wildcard *.c)
HDR := $(wildcard *.c)
all: $(SRC) $(HDR)
	gcc -w Client.c -lrt -o  Client
	gcc -w Server.c -lpthread -lncurses -o Server
	gcc -w ShmemServer.c -lpthread -lrt -lncurses -o ShmemServer
	gcc -w ShmemClient.c -lpthread -lrt -lncurses -o ShmemClient
clean:
	rm a.out