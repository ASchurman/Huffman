#-------------------------------------------------------------------------------

# Makefile for Huffman
# Alexander Schurman
# alexander.schurman@gmail.com

# target executable name
TARGET	:=huffman

# source files with extensions, separated by spaces
SOURCES	:=main.cpp node.cpp huffman.cpp

# set CPP to 1 for compiling C++; set to 0 for compiling C
CPP	:=1

# define DEBUG=1 in command line for debug
# define ARGS to be whatever arguments to pass to target

#-------------------------------------------------------------------------------

# set compilers to use--------------------
CC		:= gcc
CPPC		:= g++

ifeq ($(CPP),0)
	COMP	:= $(CC)
else
	COMP	:= $(CPPC)
endif

# flags------------------------------------
ALLFLAGS	:= -Wall -pedantic -Werror
CFLAGSBASE	:= -std=c99
CPPFLAGSBASE	:=

DEBUGFLAGS	:= -g3
RELEASEFLAGS	:= -O3

VALGRIND	:= valgrind -q --tool=memcheck --leak-check=yes

ifeq ($(DEBUG),1)
	CFLAGS	:= $(ALLFLAGS) $(CFLAGSBASE) $(DEBUGFLAGS)
	CPPFLAGS := $(ALLFLAGS) $(CPPFLAGSBASE) $(DEBUGFLAGS)
else
	CFLAGS	:= $(ALLFLAGS) $(CFLAGSBASE) $(RELEASEFLAGS)
	CPPFLAGS := $(ALLFLAGS) $(CPPFLAGSBASE) $(RELEASEFLAGS)
endif

ifeq ($(CPP),1)
	FLAGS	:= $(CPPFLAGS)
else
	FLAGS	:= $(CFLAGS)
endif

# building---------------------------------

ifeq ($(CPP),1)
	OBJ	:= $(SOURCES:.cpp=.o)
else
	OBJ	:= $(SOURCES:.c=.o)
endif

all: $(OBJ)
	$(COMP) $(FLAGS) -o $(TARGET) $^

%.o: %.c
	$(COMP) $(FLAGS) -c $<

# running----------------------------------

run: all
	./$(TARGET) $(ARGS)

time: all
	time ./$(TARGET) $(ARGS)

valgrind: all
	$(VALGRIND) ./$(TARGET) $(ARGS)
	
debug: all
	gdb ./$(TARGET) $(ARGS)

# cleaning---------------------------------

clean:
	rm -f $(TARGET) *.o
