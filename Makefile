CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2

BUILD_DIR := build

.PHONY: all cpu gpu build smoke-test clean

all: cpu gpu

# CPU programs
cpu: $(BUILD_DIR)/cpu/matmul

$(BUILD_DIR)/cpu/matmul: src/cpu/matmul.cpp examples/cpu/matmul.cpp
	@mkdir -p $(BUILD_DIR)/cpu
	$(CXX) $(CXXFLAGS) $^ -o $@

# GPU infrastructure and programs
gpu: build

build:
	@./scripts/build.sh

smoke-test: build
	@./scripts/run-smoke-test.sh

clean:
	rm -rf $(BUILD_DIR)

.PHONY: test-cpu

test-cpu: $(BUILD_DIR)/cpu/test-matmul
	./$(BUILD_DIR)/cpu/test-matmul

$(BUILD_DIR)/cpu/test-matmul: src/cpu/matmul.cpp tests/cpu/matmul.cpp
	@mkdir -p $(BUILD_DIR)/cpu
	$(CXX) $(CXXFLAGS) $^ -o $@
