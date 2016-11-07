HEADERS := $(wildcard *.h)
SRC := $(wildcard *.c)
ASM := $(wildcard *.s)
all : main
main: $(SRC) $(HEADERS)
	#gcc -S $(SRC)
	gcc $(SRC) -w -lncurses -ldl -lm -o main.out
clean:
	rm -r main.out $(ASM)
print-%: ; @echo $*=$($*)
