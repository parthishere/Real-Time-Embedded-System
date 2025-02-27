CC = gcc

FLAGS = -Werror -Wall -g

#define any include folders that contains the header files
INCLUDES = 

#include any libraries to link 
LIBS = 

#Define C source file
SRCS = lcm_invariant_schedule.c

#define C object files 
OBJS = $(SRCS:.c=.o)

# Name of binary
MAIN = program

BINARY=bin

run: $(MAIN)
	@sudo ./$(MAIN) $(ARGS)

all: clean $(MAIN) run
	@echo Program Compiled

$(MAIN): $(OBJS)
	$(CC) $(FLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm *.o $(MAIN)
