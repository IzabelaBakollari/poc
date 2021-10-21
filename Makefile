CC = gcc
CFLAGS = -Wall -g

# Just compile/link all files in one hit.
poc: poc.c
	${CC} ${CFLAGS} -o poc poc.c

clean:
	rm -f poc
