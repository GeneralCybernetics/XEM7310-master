CXX := arch -x86_64 clang++
CXXFLAGS := -std=c++11
LDFLAGS := -L./lib -lokFrontPanel
DYLD_LIBRARY_PATH := ./lib
export DYLD_LIBRARY_PATH
SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
EXECUTABLE := test

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

run:
	./$(EXECUTABLE)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean run
