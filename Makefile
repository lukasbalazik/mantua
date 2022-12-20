CC=gcc
CFLAGS=-Iinclude -Itests -lpthread -lelf
OBJ=src/mantua.c src/antidebug.c src/antivm.c src/elf_inject.c src/poly.c src/antiav.c src/aes.c
DEPS=include/manuta.h include/antidebug.h include/antivm.h include/elf_inject.h include/poly.h include/antiav.h src/aes.h
BUILD=build

TEST=tests/test.o

all: create_build $(BUILD)/test

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILD)/test: $(OBJ) $(TEST)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

create_build:
	mkdir -p $(BUILD)

clean:
	rm -f src/*.o build/test
