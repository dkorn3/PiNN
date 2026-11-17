
CXX := g++
AR := ar

CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
CPPFLAGS := -Iinclude

BUILD_DIR := build
LIB_DIR := $(BUILD_DIR)/lib
LIBRARY := $(LIB_DIR)/libpinn.a

# V3DLib dependencies
V3DLIB_DIR := third_party/V3DLib
CMDPARAM_DIR := third_party/CmdParameter
USERLAND_DIR := $(V3DLIB_DIR)/extern/userland/build/lib

# GPU include paths
GPU_CPPFLAGS := \
        -I$(V3DLIB_DIR)/Lib \
        -I$(CMDPARAM_DIR)/Lib \
        -I$(V3DLIB_DIR)/extern/bitmap \
        -I$(V3DLIB_DIR)/extern/userland/host_applications/linux/libs/bcm_host/include \
        -I$(V3DLIB_DIR)/extern/userland \
        -I$(V3DLIB_DIR)/extern/mesa/include \
        -I$(V3DLIB_DIR)/extern/mesa/src \
        -I$(V3DLIB_DIR)/extern/mesa/src/gallium/include \
        -I$(V3DLIB_DIR)/extern/mesa/src/gallium/auxiliary \
        -I$(V3DLIB_DIR)/extern/mesa/build/src

# GPU libraries
GPU_LDLIBS := \
        $(V3DLIB_DIR)/obj/debug/libv3dlib.a \
        $(CMDPARAM_DIR)/obj-debug/libCmdParameter.a \
        -L$(USERLAND_DIR) -lvcsm -pthread -lbcm_host \
        -L$(V3DLIB_DIR)/obj/debug \
        -L$(V3DLIB_DIR)/obj/mesa/bin \
        -lmesa -lstdc++fs -ldrm

.PHONY: test-gpu-relu test-activation  all cpu gpu build smoke-test clean test-cpu test-tensor run-gpu benchmark run-benchmark

# Build all examples
all: cpu gpu

# Build the PiNN static library
$(LIBRARY): $(BUILD_DIR)/tensor/tensor.o $(BUILD_DIR)/cpu/matmul.o $(BUILD_DIR)/ops/activation.o $(BUILD_DIR)/gpu/matmul.o $(BUILD_DIR)/gpu/relu.o
	@mkdir -p $(LIB_DIR)
	$(AR) rcs $@ $^

# Tensor implementation
$(BUILD_DIR)/tensor/tensor.o: src/tensor.cpp include/pinn/tensor.h
	@mkdir -p $(BUILD_DIR)/tensor
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# CPU activation implementation
$(BUILD_DIR)/ops/activation.o: src/ops/activation.cpp include/pinn/ops/activation.h include/pinn/tensor.h
	@mkdir -p $(BUILD_DIR)/ops
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# CPU implementation
$(BUILD_DIR)/cpu/matmul.o: src/cpu/matmul.cpp include/pinn/cpu/matmul.h
	@mkdir -p $(BUILD_DIR)/cpu
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# GPU implementation
$(BUILD_DIR)/gpu/matmul.o: src/gpu/matmul.cpp include/pinn/gpu/matmul.h | build
	@mkdir -p $(BUILD_DIR)/gpu
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -pthread $(GPU_CPPFLAGS) -c $< -o $@

# GPU ReLU implementation
$(BUILD_DIR)/gpu/relu.o: src/gpu/relu.cpp include/pinn/gpu/relu.h | build
	@mkdir -p $(BUILD_DIR)/gpu
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -pthread $(GPU_CPPFLAGS) -c $< -o $@

# CPU example
cpu: $(BUILD_DIR)/cpu/matmul

$(BUILD_DIR)/cpu/matmul: examples/cpu/matmul.cpp $(LIBRARY)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(LIBRARY) -o $@

# GPU example
gpu: $(BUILD_DIR)/gpu/matmul

$(BUILD_DIR)/gpu/matmul: examples/gpu/matmul.cpp $(LIBRARY)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -pthread $< $(LIBRARY) -o $@ $(GPU_LDLIBS)

# Build V3DLib and its dependencies
build:
	@./scripts/build.sh

# Dependency smoke test
smoke-test: build
	@./scripts/run-smoke-test.sh

# Tensor tests
test-tensor: $(BUILD_DIR)/tensor/test-tensor
	./$(BUILD_DIR)/tensor/test-tensor

$(BUILD_DIR)/tensor/test-tensor: tests/tensor/tensor.cpp $(BUILD_DIR)/tensor/tensor.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@

# CPU tests
test-cpu: $(BUILD_DIR)/cpu/test-matmul
	./$(BUILD_DIR)/cpu/test-matmul

$(BUILD_DIR)/cpu/test-matmul: tests/cpu/matmul.cpp $(LIBRARY)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(LIBRARY) -o $@

# Run GPU example (V3DLib currently requires root access)
run-gpu: $(BUILD_DIR)/gpu/matmul
	sudo ./$(BUILD_DIR)/gpu/matmul

# CPU vs GPU benchmark
benchmark: $(BUILD_DIR)/comparison/matmul

$(BUILD_DIR)/comparison/matmul: benchmarks/comparison/matmul.cpp $(LIBRARY)
	@mkdir -p $(BUILD_DIR)/comparison
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -pthread $< $(LIBRARY) -o $@ $(GPU_LDLIBS)

# Run CPU vs GPU benchmark (GPU requires root access)
run-benchmark: benchmark
	sudo ./$(BUILD_DIR)/comparison/matmul

# Remove generated PiNN build files
clean:
	rm -rf $(BUILD_DIR)

# Activation operators
test-activation: $(BUILD_DIR)/ops/test-activation
	./$(BUILD_DIR)/ops/test-activation

$(BUILD_DIR)/ops/test-activation: tests/ops/activation.cpp $(LIBRARY)
	@mkdir -p $(BUILD_DIR)/ops
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(LIBRARY) -o $@


# GPU ReLU tests
test-gpu-relu: $(BUILD_DIR)/gpu/test-relu
	sudo ./$(BUILD_DIR)/gpu/test-relu

$(BUILD_DIR)/gpu/test-relu: tests/gpu/relu.cpp $(LIBRARY)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -pthread $< $(LIBRARY) -o $@ $(GPU_LDLIBS)
