#-------------------------------------------------------------------------------

# Makefile for Huffman
# Alexander Schurman
# alexander.schurman@gmail.com

TARGET		:=huffman
TESTTARGET 	:=huffmanTest

SRC			:=$(shell ls *.cpp)
HEADERS		:=$(shell ls *.h)
OBJDIR		:=obj
TESTDIR		:=test
_OBJ		:=$(SRC:.cpp=.o)
OBJ			:=$(patsubst %,$(OBJDIR)/%,$(_OBJ))
TESTSRC		:=$(shell ls $(TESTDIR)/*.cpp)
_TESTOBJ	:=$(TESTSRC:.cpp=.o)
TESTOBJ		:=$(patsubst $(TESTDIR)/%,$(OBJDIR)/%,$(_TESTOBJ))

CXX			:=g++
CXXFLAGS	+=-Wall -pedantic -Werror -std=c++17

ifeq ($(DEBUG),1)
	CXXFLAGS+= $(ALLFLAGS) -ggdb3
else
	CXXFLAGS+= $(ALLFLAGS)
endif

.PHONY: all
all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.cpp $(OBJDIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<
$(OBJDIR)/%.o: $(TESTDIR)/%.cpp $(OBJDIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.PHONY: test
test: $(filter-out $(OBJDIR)/main.o,$(OBJ)) $(TESTOBJ)
	$(CXX) $(CXXFLAGS) -o $(TESTTARGET) $^

# running----------------------------------

.PHONY: run
run: all
	./$(TARGET) $(ARGS)

.PHONY: time
time: all
	time ./$(TARGET) $(ARGS)

.PHONY: debug
debug: all
	gdb ./$(TARGET) $(ARGS)

# cleaning---------------------------------

.PHONY: clean
clean:
	rm -rf $(TARGET) $(TESTTARGET) $(OBJDIR)
