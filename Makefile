CFLAGSSS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

toycc: $(OBJS)
	$(CC) -Wall -o toycc $(OBJS) $(LDFLAGS)

$(OBJS): toycc.h

test: toycc
	./test.sh

clean:
	rm -f toycc *.o *~ tmp*

.PHONY: test clean
