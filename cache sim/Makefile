# Makefile for Direct-Mapped Cache Simulator
# Compiles the cache simulator and test suite

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
INCLUDES = -I. -I./src -I./memory\ system\ simulator/c++

# Source files
CACHE_SOURCES = src/cache/direct_mapped_cache.cpp
STATS_SOURCES = memory\ system\ simulator/c++/statistics.cpp
TEST_SOURCES = tests/test_direct_mapped.cpp

# Object files
CACHE_OBJS = $(CACHE_SOURCES:.cpp=.o)
STATS_OBJS = statistics.o
TEST_OBJS = $(TEST_SOURCES:.cpp=.o)

# Executables
TEST_EXEC = test_cache

.PHONY: all clean test

all: $(TEST_EXEC)

# Build test executable
$(TEST_EXEC): $(TEST_OBJS) $(CACHE_OBJS) $(STATS_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "✓ Built test executable: $(TEST_EXEC)"

# Compile cache implementation
src/cache/direct_mapped_cache.o: src/cache/direct_mapped_cache.cpp src/cache/direct_mapped_cache.h src/cache/cache_line.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile statistics (from memory system simulator)
statistics.o: memory\ system\ simulator/c++/statistics.cpp memory\ system\ simulator/c++/statistics.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile test file
tests/test_direct_mapped.o: tests/test_direct_mapped.cpp src/cache/direct_mapped_cache.h src/cache/cache_line.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Run tests
test: $(TEST_EXEC)
	@echo "\n======================================="
	@echo "Running Cache Simulator Tests"
	@echo "======================================="
	./$(TEST_EXEC)

# Clean build artifacts
clean:
	rm -f $(CACHE_OBJS) $(STATS_OBJS) $(TEST_OBJS) $(TEST_EXEC) statistics.o
	@echo "✓ Cleaned build artifacts"

# Help target
help:
	@echo "Direct-Mapped Cache Simulator - Build Targets:"
	@echo "  make all     - Build test executable"
	@echo "  make test    - Build and run tests"
	@echo "  make clean   - Remove build artifacts"
	@echo "  make help    - Show this help message"
