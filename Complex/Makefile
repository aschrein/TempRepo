
MODULES_SRC := $(wildcard plugins/*.c)
OBJECTS := $(patsubst %.c, %.o, $(MODULES_SRC))
MODULES := $(patsubst %.c, %.so, $(MODULES_SRC))
HEADERS := $(wildcard)
all : main
main : main.c $(MODULES)
	gcc main.c -w -lX11 -ldl -o main
%.so : %.o
	gcc -shared  -o $@ $<
%.o : %.c $(HEADERS)
	gcc -w -Wl,--hash-style=sysv -fPIC -Iinclude -c -o $@ $<
clean:
	rm -f *.o *.a plugins/*.o plugins/*.so main
print-%: ; @echo $*=$($*)
