CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11
LDFLAGS = -lncurses

TARGET = sudoku_solver
SRCS = main.cpp sudoku.cpp generatepuzzle.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
