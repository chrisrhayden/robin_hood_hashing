CC=gcc
CFLAGS=-Wall -ggdb3 -pedantic

SRC = $(wildcard ./src/*.c)

TARGET_NAME = ./out/target_build

build: $(SRC)
	$(CC) $(CFLAGS) $^ -o $(TARGET_NAME)

run: build
	./out/target_build
