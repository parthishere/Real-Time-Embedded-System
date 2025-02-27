INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= a.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	a

clean:
	-rm -f *.o *.d
	-rm -f a

a: a.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lm

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
