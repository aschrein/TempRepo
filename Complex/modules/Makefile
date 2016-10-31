OUT_DIR := ../build
TMP_DIR := tmp
INCLUDE_DIR := ../include
HEADERS := $(wildcard $(INCLUDE_DIR)/*.h)
MODULES_DIR := $(OUT_DIR)/modules
MODULES_SRC := $(wildcard *.c)
OBJECTS := $(patsubst %.c, $(TMP_DIR)/%.o, $(MODULES_SRC))
PLUGINS := $(patsubst %.c, $(MODULES_DIR)/%.so, $(MODULES_SRC))
DEP_FILES := $(wildcard *.dep)
DST_DEP_FILES := $(patsubst %, $(MODULES_DIR)/%, $(DEP_FILES))
#HEADERS := $(wildcard include/*.h)
all : $(PLUGINS) $(DST_DEP_FILES)
$(DST_DEP_FILES): $(MODULES_DIR)/%: %
	cp -T $< $@
$(TMP_DIR):
	mkdir -p $@
$(MODULES_DIR):
	mkdir -p $@
$(PLUGINS): $(MODULES_DIR)/%.so: $(TMP_DIR)/%.o $(MODULES_DIR)
	gcc -shared -o $@ $<
$(OBJECTS): $(TMP_DIR)/%.o: %.c $(HEADERS)  $(TMP_DIR)
	gcc -w -Wl,--hash-style=sysv -fpic -I$(INCLUDE_DIR) -c -o $@ $<
clean:
	rm -r $(TMP_DIR) $(MODULES_DIR)
print-%: ; @echo $*=$($*)
