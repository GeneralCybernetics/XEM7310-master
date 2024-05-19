CXX := g++
CXXFLAGS := -std=c++11
LDFLAGS := -L./lib -lokFrontPanel
LD_LIBRARY_PATH := ./lib
export LD_LIBRARY_PATH
SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
EXECUTABLE := test

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

run: $(EXECUTABLE)
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) ./$(EXECUTABLE)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean run
