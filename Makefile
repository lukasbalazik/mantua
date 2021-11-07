CC=gcc
CFLAGS=-Wall -O3 -Ofast -Iinclude -Itests -lpthread -lelf
OBJ=src/mantua.c src/antidebug.c  src/elf_inject.c
DEPS=include/manuta.h include/antidebug.h include/elf_inject.h
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
