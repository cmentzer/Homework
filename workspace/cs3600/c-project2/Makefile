CFILES = $(wildcard *.c)

cp%: cp%.c
	gcc -std=c99 -O0 -g -lm -Wall -pedantic -Werror -o $@ $<

all: $(basename $(CFILES))

test: all
	./test $(basename $(CFILES))

clean:
	rm $(basename $(CFILES))
