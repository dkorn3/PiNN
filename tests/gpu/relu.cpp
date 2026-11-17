#include <pinn/gpu/relu.h>
#include <pinn/tensor.h>

#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>

bool test_size(std::size_t size) {
    std::vector<float> input(size);
    std::vector<float> output(size, -999.0f);

    for (std::size_t i = 0; i < size; ++i) {
        input[i] = static_cast<float>(
            static_cast<int>(i % 11) - 5
        );
    }

    pinn::gpu::relu(input.data(), output.data(), size);

    for (std::size_t i = 0; i < size; ++i) {
        const float expected = input[i] > 0.0f ? input[i] : 0.0f;

        if (std::fabs(output[i] - expected) > 1e-6f) {
            std::cerr << "FAIL: size " << size
                      << ", index " << i
                      << ", expected " << expected
                      << ", got " << output[i] << '\n';
            return false;
        }
    }

    std::cout << "PASS: size " << size << '\n';
    return true;
}

bool test_tensor_in_place() {
    pinn::Tensor tensor({17});

    for (std::size_t i = 0; i < tensor.size(); ++i) {
        tensor[i] = static_cast<float>(
            static_cast<int>(i % 7) - 3
        );
    }

    pinn::gpu::relu(tensor);

    for (std::size_t i = 0; i < tensor.size(); ++i) {
        const int value = static_cast<int>(i % 7) - 3;
        const float expected = value > 0 ? static_cast<float>(value) : 0.0f;

        if (std::fabs(tensor[i] - expected) > 1e-6f) {
            std::cerr << "FAIL: Tensor ReLU at index " << i << '\n';
            return false;
        }
    }

    std::cout << "PASS: Tensor in-place ReLU\n";
    return true;
}

int main() {
    int failures = 0;
    int tests = 0;

    for (const std::size_t size : {
        1, 3, 15, 16, 17, 31, 32, 33, 64, 127, 256, 1024
    }) {
        ++tests;
        failures += !test_size(size);
    }

    ++tests;
    failures += !test_tensor_in_place();

    std::cout << "\nTests passed: "
              << tests - failures << "/" << tests << '\n';

    return failures == 0 ? 0 : 1;
}
