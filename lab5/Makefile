CC=gcc
CFLAGS=-g -pthread -W -Wall -Wno-unused-parameter -Wno-unused-variable -pedantic
BIN_DIR=bin

all: main

main: src/main.c src/ring.c src/func.c src/stack.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $^

clean:
	rm -rf $(BIN_DIR)