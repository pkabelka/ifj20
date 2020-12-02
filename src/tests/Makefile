CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g -DDEBUG
LDFLAGS=

all: scanner_test

.PHONY: clean run

clean:
	rm -rf scanner_test

run: all
	./scanner_test num < scanner/num.txt && ./scanner_test factorial < scanner/factorial.go

scanner_test:
	cp -f -t . ../scanner.h ../scanner.c ../str.h ../str.c ../error.h ../stack.c ../stack.h ../symtable.h ../symtable.c
	$(CC) $(CFLAGS) optimizer_test.c scanner_test.c scanner.h scanner.c str.h str.c error.h stack.c stack.h symtable.h symtable.c -o scanner_test $(LDFLAGS)
