BINARY_NAME=slow

# C compiler and flags
CC=clang
CFLAGS=-std=c99 -Wall -Wextra -O3 -D_POSIX_C_SOURCE=200809L
LDFLAGS=-static

# Source files
SRC=main.c

# Git tag for versioning
GIT_TAG := $(shell git describe --tags --always 2>/dev/null || echo "unknown")
CFLAGS += -DGIT_TAG=\"$(GIT_TAG)\"

.PHONY: all build build-cross clean test

all: build

build:
	# Build for the current OS and architecture
	$(CC) $(CFLAGS) $(SRC) -o $(BINARY_NAME)

test: build
	# Basic functionality tests
	@echo "Running basic tests..."
	@echo "Hello World" | ./$(BINARY_NAME) -b 56000 > /dev/null && echo "Test 1: PASSED" || echo "Test 1: FAILED"
	@./$(BINARY_NAME) -h > /dev/null && echo "Test 2: PASSED" || echo "Test 2: FAILED"
	@echo "Testing preset..." | ./$(BINARY_NAME) -b dialup > /dev/null && echo "Test 3: PASSED" || echo "Test 3: FAILED"

clean:
	rm -f $(BINARY_NAME) $(BINARY_NAME)-*

