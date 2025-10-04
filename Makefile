# Alternative Makefile for systems without CMake
# Requires LLVM to be installed with llvm-config available

CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-unused-parameter
LLVM_CONFIG = llvm-config

# Get LLVM compilation flags
LLVM_CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags)
LLVM_LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags)
LLVM_LIBS := $(shell $(LLVM_CONFIG) --libs core support irreader analysis executionengine instcombine object runtimedyld scalaropts transformutils)

# Combine flags
CXXFLAGS += $(LLVM_CXXFLAGS)
LDFLAGS = $(LLVM_LDFLAGS) $(LLVM_LIBS)

# Source files
SOURCES = main.cpp ast.cpp lexer.cpp parser.cpp codegen.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = clike-compiler

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Install target (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Uninstall target (optional)
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Debug build
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)

# Release build
release: CXXFLAGS += -O3 -DNDEBUG
release: $(TARGET)

.PHONY: all clean install uninstall debug release