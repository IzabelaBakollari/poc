.PHONY: clean

poc: poc.c
	gcc -Wall -Werror -g -o $@ $<

clean:
	rm poc
