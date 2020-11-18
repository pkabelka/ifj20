CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g -DDEBUG
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
	rm -rf $(obj) $(BIN) ../$(PACK).tgz

# run with: make run ARGS="some arguments"
run: all
	./$(BIN) $(ARGS)

../$(PACK).tgz: $(src) $(headers)
	tar -czvf $@ $^ Makefile ../rozdeleni

pack: clean ../$(PACK).tgz
