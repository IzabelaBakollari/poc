CC = gcc
CFLAGS = -Wall -g

poc.o: poc.c  functions.h
	${CC} ${CFLAGS} -c poc.c

txsetup.o: txsetup.c functions.h
	${CC} ${CFLAGS}  -c txsetup.c

rxsetup.o: rxsetup.c  functions.h
	${CC} ${CFLAGS} -c  rxsetup.c
	
rxchanged.o: rxsetup.c  functions.h
	${CC} ${CFLAGS} -c  rxchanged.c


check.o: check.c functions.h
	${CC} ${CFLAGS}  -c check.c


poc: txsetup.o rxsetup.o rxchanged.o check.o poc.o functions.h
	$(CC) txsetup.o rxsetup.o rxchanged.o check.o poc.o -o poc
