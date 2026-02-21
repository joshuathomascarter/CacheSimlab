# ============================================================================
# SRAM Array - Behavioral Model + RTL Cross-Validation
# Build system for testing SRAM controller
# ============================================================================

CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -g
LDFLAGS := -lm

# Directories
SRC_DIR := cpp
RTL_DIR := rtl
TEST_DIR := tests
BUILD_DIR := build
OUTPUT_DIR := ../../../output/sram

# Source files
CPP_SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
TEST_SOURCES := $(wildcard $(TEST_DIR)/*.cpp)

# Object files
CPP_OBJECTS := $(CPP_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
TEST_OBJECTS := $(filter-out $(BUILD_DIR)/$(SRC_DIR)/main.o, $(CPP_OBJECTS))

# Executables
SRAM_MODEL := $(BUILD_DIR)/test_sram_model
SRAM_TEST := $(BUILD_DIR)/test_sram_controller

# ============================================================================
# Targets
# ============================================================================

.PHONY: all test clean help

all: $(SRAM_MODEL) $(SRAM_TEST)

test: $(SRAM_TEST)
	@echo ""
	@echo "╔════════════════════════════════════════════════════════╗"
	@echo "║              SRAM TESTBENCH EXECUTION                  ║"
	@echo "╚════════════════════════════════════════════════════════╝"
	@echo ""
	$(SRAM_TEST)
	@echo "✓ TESTBENCH COMPLETE"

test_model: $(SRAM_MODEL)
	@echo ""
	@echo "╔════════════════════════════════════════════════════════╗"
	@echo "║           C++ BEHAVIORAL MODEL VALIDATION              ║"
	@echo "╚════════════════════════════════════════════════════════╝"
	@echo ""
	$(SRAM_MODEL)
	@echo "✓ MODEL VALIDATION COMPLETE"

# ============================================================================
# Build Rules
# ============================================================================

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)
	@mkdir -p $(BUILD_DIR)/$(TEST_DIR)
	@mkdir -p $(OUTPUT_DIR)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "  [CC] $<"

$(SRAM_MODEL): $(CPP_OBJECTS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPP_OBJECTS) -o $@ $(LDFLAGS)
	@echo ""
	@echo "  ✓ Built: $@"

$(SRAM_TEST): $(TEST_OBJECTS) $(BUILD_DIR)/$(TEST_DIR)/tb_sram_controller.o | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_OBJECTS) $(BUILD_DIR)/$(TEST_DIR)/tb_sram_controller.o -o $@ $(LDFLAGS)
	@echo ""
	@echo "  ✓ Built: $@"

# ============================================================================
# Utility Targets
# ============================================================================

help:
	@echo ""
	@echo "SRAM Array Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build all executables (default)"
	@echo "  test         - Run SRAM controller testbench"
	@echo "  test_model   - Run behavioral model validation"
	@echo "  clean        - Remove all build artifacts"
	@echo "  help         - Show this message"
	@echo ""

clean:
	@rm -rf $(BUILD_DIR)
	@echo "✓ Cleaned build artifacts"

# ============================================================================
# Info Targets
# ============================================================================

info:
	@echo ""
	@echo "SRAM Array Build Configuration"
	@echo "=============================="
	@echo "Compiler:       $(CXX)"
	@echo "C++ Standard:   c++17"
	@echo "Optimization:   -O2"
	@echo ""
	@echo "C++ Sources:    $(CPP_SOURCES)"
	@echo "Test Sources:   $(TEST_SOURCES)"
	@echo ""
	@echo "Build Dir:      $(BUILD_DIR)"
	@echo "Output Dir:     $(OUTPUT_DIR)"
	@echo ""

.DEFAULT_GOAL := all
