CC = g++
CFLAGS = -std=c++17 -Wall -I./headers

TEST_SRC = tests/test_bank_fsm.cpp cpp/dram_bank_fsm.cpp cpp/timing_validator.cpp
TEST_OBJ = $(TEST_SRC:.cpp=.o)
TEST_BIN = tests/test_bank_fsm

all: $(TEST_BIN)

$(TEST_BIN): $(TEST_SRC)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f tests/*.o cpp/*.o $(TEST_BIN)

run_tests: all
	./$(TEST_BIN)
