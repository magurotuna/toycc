CFLAGSSS=-std=c11 -g -static

toycc: toycc.c

test: toycc
	./test.sh

clean:
	rm -f toycc *.o *~ tmp*

.PHONY: test clean
