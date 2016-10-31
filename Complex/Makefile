HEADERS := $(wildcard include/*.h)
TMP_DIR := tmp
SRC_DIR := src
OUT_DIR := build
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(TMP_DIR)/%.o, $(SRC))
all : directories main plugins
	cd build && ./main
directories: $(TMP_DIR) $(OUT_DIR)
$(TMP_DIR):
	mkdir $@
$(OUT_DIR):
	mkdir $@
plugins: Complex.c Src2Mod.py
	python Src2Mod.py Complex.c
	cd modules && make
main: $(OBJ) $(HEADERS)
	gcc $(OBJ) -w -lX11 -ldl -lm -o $(OUT_DIR)/main
$(OBJ): $(TMP_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	gcc -w -Wl,--hash-style=sysv -Iinclude -c -o $@ $<
clean:
	rm -r $(TMP_DIR)  $(OUT_DIR)/main
	cd modules && make clean
	cd modules && find . -name '*.c' -delete
	cd modules && find . -name '*.dep' -delete
	rm -r $(OUT_DIR)
print-%: ; @echo $*=$($*)
