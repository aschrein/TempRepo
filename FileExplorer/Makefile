HEADERS := $(wildcard *.h)
SRC := $(wildcard *.c)
ASM := $(wildcard *.s)
all : main
main: $(SRC) $(HEADERS)
	gcc Explorer.c  text.c -w -lncurses -ldl -lm -o Explorer.out
	gcc Editor.c Event.c text.c -w -lncurses -ldl -lm -o Editor.out
	gcc Coppy.c  -w -lncurses -o Coppy.out
clean:
	rm -r *.out $(ASM)
print-%: ; @echo $*=$($*)
