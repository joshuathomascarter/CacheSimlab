.PHONY: all build-4way run-4way build-direct run-direct py venv install-reqs test clean format

ROOT := $(shell pwd)
FOURWAY_DIR := $(ROOT)/"cache sim/4-way cache"
DIRECT_DIR := $(ROOT)/"cache sim/direct-way"

# Default target builds everything
all: test py

# Build and run tests for 4-way cache
build-4way:
	mkdir -p build/4way && cd build/4way && cmake "$(FOURWAY_DIR)" && make

run-4way: build-4way
	cd build/4way && ./test_cache

# Build and run tests for direct-mapped cache
build-direct:
	mkdir -p build/direct && cd build/direct && cmake "$(DIRECT_DIR)" && make

run-direct: build-direct
	cd build/direct && ./test_cache

# Python analysis targets
venv:
	python -m venv .venv

install-reqs: venv
	.venv/bin/python -m pip install --upgrade pip setuptools wheel
	.venv/bin/python -m pip install -r "cache sim/4-way cache/python/analysis/requirements.txt"

py: install-reqs
	.venv/bin/python "cache sim/4-way cache/python/analysis/working_set.py"
	.venv/bin/python "cache sim/4-way cache/python/analysis/reuse_distance.py"

# Run all tests and analyses
test: run-4way run-direct py

# Format code
format:
	find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i

clean:
	rm -rf build/
	rm -f working_set_analysis.png reuse_distance_analysis.png
