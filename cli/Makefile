CXX      ?= c++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -O2
PREFIX   ?= $(HOME)/.local

TARGET  := bin/kit
SRCS    := $(wildcard src/*.cpp)
OBJS    := $(patsubst src/%.cpp,build/%.o,$(SRCS))

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

install: $(TARGET)
	@mkdir -p $(PREFIX)/bin
	install -m 755 $(TARGET) $(PREFIX)/bin/kit
	@echo "installed to $(PREFIX)/bin/kit"

uninstall:
	rm -f $(PREFIX)/bin/kit

clean:
	rm -rf build bin
