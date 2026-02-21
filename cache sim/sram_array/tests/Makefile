# SRAM Controller Testbench Makefile
# Supports both Verilator RTL simulation and standalone C++ golden model testing

# Paths
RTL_DIR = ../rtl
CPP_DIR = ../cpp
TEST_DIR = .

# Source files
RTL_SOURCES = $(RTL_DIR)/sram_controller.v $(RTL_DIR)/sram_array.v
CPP_TESTBENCH = tb_sram_controller.cpp
CPP_GOLDEN_MODEL = $(CPP_DIR)/sram_behavioral_model.cpp

# Verilator configuration
VERILATOR = verilator
VERILATOR_FLAGS = --cc --exe --build -Wall
VERILATOR_TOP = sram_controller

# C++ compiler for standalone mode
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -I$(CPP_DIR)
LDFLAGS =

# Targets
.PHONY: all rtl standalone clean help

all: help

# Build with Verilator (full RTL simulation)
rtl: $(RTL_SOURCES) $(CPP_TESTBENCH) $(CPP_GOLDEN_MODEL)
	@echo "════════════════════════════════════════════════════════"
	@echo "Building SRAM testbench with Verilator RTL simulation"
	@echo "Linking against REAL golden model (no duplicates!)"
	@echo "════════════════════════════════════════════════════════"
	$(VERILATOR) $(VERILATOR_FLAGS) \
		--Mdir obj_dir_rtl \
		--top-module $(VERILATOR_TOP) \
		$(RTL_SOURCES) \
		$(CPP_TESTBENCH) \
		$(CPP_GOLDEN_MODEL) \
		-CFLAGS "-DVERILATOR -I$(CPP_DIR)"
	@echo ""
	@echo "✓ Build complete! Run with: ./obj_dir_rtl/V$(VERILATOR_TOP)"
	@echo "✓ Using SINGLE authoritative golden model"
	@echo ""

# Build standalone C++ golden model (no RTL/Verilator)
standalone: $(CPP_TESTBENCH) $(CPP_GOLDEN_MODEL)
	@echo "════════════════════════════════════════════════════════"
	@echo "Building standalone testbench (C++ golden model only)"
	@echo "════════════════════════════════════════════════════════"
	$(CXX) $(CXXFLAGS) -o test_golden_model \
		$(CPP_TESTBENCH) \
		$(CPP_GOLDEN_MODEL)
	@echo ""
	@echo "✓ Build complete! Run with: ./test_golden_model"
	@echo "  (Note: This tests golden model only, not RTL)"
	@echo ""

# Run RTL simulation
run_rtl: rtl
	@echo "════════════════════════════════════════════════════════"
	@echo "Running RTL cross-validation testbench"
	@echo "════════════════════════════════════════════════════════"
	./obj_dir_rtl/V$(VERILATOR_TOP)

# Run standalone golden model test
run_standalone: standalone
	@echo "════════════════════════════════════════════════════════"
	@echo "Running standalone C++ golden model test"
	@echo "════════════════════════════════════════════════════════"
	./test_golden_model

# Clean build artifacts
clean:
	rm -rf obj_dir_rtl/
	rm -f test_golden_model
	rm -f *.vcd *.csv
	@echo "✓ Cleaned build artifacts"

# Help target
help:
	@echo ""
	@echo "SRAM Controller Testbench Build System"
	@echo "══════════════════════════════════════════════════════════════"
	@echo ""
	@echo "Targets:"
	@echo "  make rtl            - Build with Verilator (tests actual RTL)"
	@echo "  make run_rtl        - Build + run RTL simulation"
	@echo "  make standalone     - Build golden model only (no RTL)"
	@echo "  make run_standalone - Build + run golden model test"
	@echo "  make clean          - Remove build artifacts"
	@echo "  make help           - Show this help"
	@echo ""
	@echo "What's the difference?"
	@echo "  RTL mode:        Tests Verilog RTL against C++ golden model"
	@echo "                   → Full cross-validation (what you want!)"
	@echo "                   → Requires Verilator installed"
	@echo ""
	@echo "  Standalone mode: Tests C++ golden model only"
	@echo "                   → Quick sanity check"
	@echo "                   → Doesn't test RTL"
	@echo ""
	@echo "Quick Start:"
	@echo "  make run_rtl        # Most common - full RTL validation"
	@echo ""
	@echo "Prerequisites:"
	@echo "  - Verilator: brew install verilator  (macOS)"
	@echo "              sudo apt install verilator (Linux)"
	@echo ""
