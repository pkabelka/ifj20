CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -Werror -pedantic
LDFLAGS=
src=$(wildcard *.c)
obj=$(src:.c=.o)
headers=$(wildcard *.h)
BIN=ifj20

all: $(BIN)
$(BIN): $(obj) $(headers)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

.PHONY: clean run

clean:
	rm -rf $(obj) $(BIN)

run: all
	./$(BIN)
