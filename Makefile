CC = gcc
CFLAGS = -Wall -g

poc: poc.o
	$(CC) poc.o -o poc

poc.o: poc.c
	${CC} ${CFLAGS} -c poc.c

clean:
	rm -rf poc.o poc
