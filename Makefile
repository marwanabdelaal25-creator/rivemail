CC = cc
CFLAGS ?= -std=c11 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude -Isrc
FASTCFLAGS ?= -std=c11 -g0 -O0 -fno-omit-frame-pointer -Iinclude -Isrc
LDFLAGS ?= -fsanitize=address,undefined
SRC := $(wildcard src/*.c)
LIBSRC := $(filter-out src/cli.c,$(SRC))
OBJ := $(LIBSRC:.c=.o)

.PHONY: all test clean tool fuzz
all: tool

src/analysis_mime.o: src/analysis_mime.c
	$(CC) $(CFLAGS) -c $< -o $@

src/analysis_%.o: src/analysis_%.c
	$(CC) $(FASTCFLAGS) -c $< -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

tests/test_main: tests/test_main.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

rivemail: $(SRC)
	$(CC) $(CFLAGS) -DRIVEMAIL_STANDALONE $(SRC) -o $@ $(LDFLAGS)

tool: rivemail

test: tests/test_main
	./tests/test_main

fuzz:
	mkdir -p build/out
	SRC=$$(pwd) OUT=$$(pwd)/build/out LIB_FUZZING_ENGINE='-fsanitize=fuzzer' bash .clusterfuzzlite/build.sh

clean:
	rm -f src/*.o tests/test_main rivemail
	rm -rf build/out
