SRC := $(wildcard *.c)
HDR := $(wildcard *.c)
all: $(SRC) $(HDR)
	gcc -w Server.c -lrt -lncurses -o Server
	gcc -w Client.c -lrt -o Client
	gcc -w ShmemServer.c -lpthread -lrt -lncurses -o ShmemServer
	gcc -w ShmemClient.c -lrt -lpthread -o ShmemClient
clean:
	rm *.out