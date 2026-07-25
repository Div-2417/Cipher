# Gotya 2.0 Chess Engine Makefile

CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
TARGET := gotya
SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)

.PHONY: all build clean run help

all: build

build: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: build
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

help:
	@echo "Gotya 2.0 Chess Engine - Makefile Commands"
	@echo "  make build    - Build the engine"
	@echo "  make run      - Build and run the engine"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make help     - Show this help message"
