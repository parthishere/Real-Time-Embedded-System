CC = gcc

CFLAGS = -g

#define any include folders that contains the header files
INCLUDES = 

#include any libraries to link 
LIBS = -lpthread -lrt

#Define C source file
SRCS = simple.c

#define C object files 
OBJS = $(SRCS:.c=.o)

# Name of binary
MAIN = program

BINARY=bin

run: $(MAIN)
	@echo Runing Executable
	@sudo ./$(MAIN) $(ARGS)

all: clean $(MAIN) run remove
	@echo Program Compiled

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS) -lm

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ -lm

clean:
	@echo Cleaning all files
	-rm *.o $(MAIN)

remove:
	@echo Cleaning .o files
	-rm *.o
