CC = gcc
CFLAGS = -Wall -g

# Just compile/link all files in one hit.
demo: poc.c
	${CC} ${CFLAGS} -o poc poc.c

clean:
	rm -f poc
