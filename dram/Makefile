CC = g++
CFLAGS = -std=c++17 -Wall -I./headers

# Day 1: Bank FSM test
BANK_TEST_SRC = bin/test_bank_fsm.cpp cpp/dram_bank_fsm.cpp cpp/timing_validator.cpp
BANK_TEST_OBJ = $(BANK_TEST_SRC:.cpp=.o)
BANK_TEST_BIN = bin/test_bank_fsm

# Day 2: Refresh power test
REFRESH_TEST_SRC = bin/test_refresh_power.cpp cpp/power_model.cpp cpp/refresh_controller.cpp cpp/dram_bank_fsm.cpp cpp/timing_validator.cpp
REFRESH_TEST_OBJ = $(REFRESH_TEST_SRC:.cpp=.o)
REFRESH_TEST_BIN = bin/test_refresh_power

# Day 3: Scheduling test
SCHED_TEST_SRC = tests/test_scheduling.cpp cpp/memory_channel.cpp cpp/memory_scheduler.cpp cpp/dram_bank_fsm.cpp cpp/timing_validator.cpp
SCHED_TEST_OBJ = $(SCHED_TEST_SRC:.cpp=.o)
SCHED_TEST_BIN = bin/test_scheduling

# Day 3: Hybrid scheduler example
HYBRID_EXAMPLE_SRC = tests/example_hybrid_usage.cpp cpp/memory_channel.cpp cpp/memory_scheduler.cpp cpp/dram_bank_fsm.cpp cpp/timing_validator.cpp
HYBRID_EXAMPLE_BIN = bin/example_hybrid

all: $(BANK_TEST_BIN) $(REFRESH_TEST_BIN) $(SCHED_TEST_BIN) $(HYBRID_EXAMPLE_BIN)

$(BANK_TEST_BIN): $(BANK_TEST_SRC)
	$(CC) $(CFLAGS) $^ -o $@

$(REFRESH_TEST_BIN): $(REFRESH_TEST_SRC)
	$(CC) $(CFLAGS) $^ -o $@

$(SCHED_TEST_BIN): $(SCHED_TEST_SRC)
	$(CC) $(CFLAGS) $^ -o $@

$(HYBRID_EXAMPLE_BIN): $(HYBRID_EXAMPLE_SRC)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f tests/*.o cpp/*.o bin/test_bank_fsm bin/test_refresh_power bin/test_scheduling bin/example_hybrid

run_bank_tests: $(BANK_TEST_BIN)
	./$(BANK_TEST_BIN)

run_refresh_tests: $(REFRESH_TEST_BIN)
	./$(REFRESH_TEST_BIN)

run_scheduling_tests: $(SCHED_TEST_BIN)
	./$(SCHED_TEST_BIN)

run_hybrid_example: $(HYBRID_EXAMPLE_BIN)
	./$(HYBRID_EXAMPLE_BIN)

run_tests: run_bank_tests run_refresh_tests run_scheduling_tests

# Day 3 specific targets
day3: $(SCHED_TEST_BIN) $(HYBRID_EXAMPLE_BIN)
	@echo "Building Day 3: Multi-Bank Coordination & Scheduling..."
	./$(SCHED_TEST_BIN)
	@echo "\n========================================="
	@echo "Running HYBRID scheduler examples..."
	@echo "=========================================\n"
	./$(HYBRID_EXAMPLE_BIN)
	@echo "\nRunning scheduling analysis..."
	cd python && python3 scheduling_analyzer.py
