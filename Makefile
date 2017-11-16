CXX=g++
CXXFLAGS=-g -std=c++11 -Wall -pedantic -Wno-vla-extension
BIN=main

SRC=$(wildcard *.cpp)
OBJ=$(SRC:%.cpp=%.o)

#if optimize:
CXXFLAGS += -O3

all: $(OBJ)
	$(CXX) -o $(BIN) $^

$(BIN): $(OBJ)
	$(CXX) -o $(BIN) $^

%.o: %.c
	$(CXX) $(CXXFLAGS) $@ -c $<

clean:
	rm -f *.o
	rm $(BIN)

run: $(BIN)
	./$(BIN)
