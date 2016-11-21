SRC := $(wildcard *.c)
HDR := $(wildcard *.c)
all: $(SRC) $(HDR)
	gcc -w Client.c -lpthread -o  Client
	gcc -w Server.c -lpthread -o Server
	gcc -w Test.c -lpthread -o Test
clean:
	rm a.out
