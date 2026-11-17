#include <pinn/ops/activation.h>

#include <cmath>
#include <iostream>

bool check(
    const pinn::Tensor& tensor,
    const float* expected,
    const char* name
) {
    for (std::size_t i = 0; i < tensor.size(); ++i) {
        if (std::fabs(tensor[i] - expected[i]) > 1e-6f) {
            std::cerr << "FAIL: " << name
                      << " at index " << i
                      << ", expected " << expected[i]
                      << ", got " << tensor[i] << '\n';
            return false;
        }
    }

    std::cout << "PASS: " << name << '\n';
    return true;
}

int main() {
    int failures = 0;

    // Mixed positive, negative, and zero values.
    {
        pinn::Tensor tensor({5});
        const float input[] = {-3, -1, 0, 2, 4};
        const float expected[] = {0, 0, 0, 2, 4};

        for (std::size_t i = 0; i < 5; ++i) {
            tensor[i] = input[i];
        }

        pinn::ops::relu(tensor);
        failures += !check(tensor, expected, "Mixed values");
    }

    // All-negative values.
    {
        pinn::Tensor tensor({3});
        const float expected[] = {0, 0, 0};

        tensor[0] = -1;
        tensor[1] = -2;
        tensor[2] = -3;

        pinn::ops::relu(tensor);
        failures += !check(tensor, expected, "All-negative values");
    }

    // All-positive values.
    {
        pinn::Tensor tensor({3});
        const float expected[] = {1, 2, 3};

        tensor[0] = 1;
        tensor[1] = 2;
        tensor[2] = 3;

        pinn::ops::relu(tensor);
        failures += !check(tensor, expected, "All-positive values");
    }

    // Four-dimensional tensor.
    {
        pinn::Tensor tensor({1, 2, 2, 2});

        const float input[] = {
            -1, 2, -3, 4,
            5, -6, 7, -8
        };

        const float expected[] = {
            0, 2, 0, 4,
            5, 0, 7, 0
        };

        for (std::size_t i = 0; i < 8; ++i) {
            tensor[i] = input[i];
        }

        pinn::ops::relu(tensor);
        failures += !check(tensor, expected, "4D tensor");
    }

    std::cout << "\nTests passed: "
              << 4 - failures << "/4\n";

    return failures == 0 ? 0 : 1;
}
