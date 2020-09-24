CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -Werror -pedantic
LDFLAGS=
src=$(wildcard *.c)
obj=$(src:.c=.o)
headers=$(wildcard *.h)
BIN=ifj20
PACK=xkabel09

all: $(BIN)
$(BIN): $(obj) $(headers)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

.PHONY: clean run pack

clean:
	rm -rf $(obj) $(BIN) $(PACK).tgz

run: all
	./$(BIN)

$(PACK).tgz: $(src) $(headers)
	tar -czvf $@ $^ Makefile rozdeleni

pack: clean $(PACK).tgz
