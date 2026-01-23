#!/bin/bash

echo "Cleaning up..."
make clean

echo "Compiling tests..."
make
if [ $? -ne 0 ]; then
    echo "Compilation Failed!"
    exit 1
fi

echo "Running Bank FSM Tests..."
./tests/test_bank_fsm

if [ $? -eq 0 ]; then
    echo "SUCCESS: All tests passed!"
else
    echo "FAILURE: Some tests failed."
    exit 1
fi
