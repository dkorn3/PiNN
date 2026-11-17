#include <pinn/tensor.h>

#include <iostream>
#include <stdexcept>
#include <vector>

bool check(bool condition, const char* name) {
    if (!condition) {
        std::cerr << "FAIL: " << name << '\n';
        return false;
    }

    std::cout << "PASS: " << name << '\n';
    return true;
}

int main() {
    int failures = 0;

    // 2D matrix.
    {
        pinn::Tensor tensor({2, 3});

        failures += !check(tensor.rank() == 2, "2D rank");
        failures += !check(tensor.size() == 6, "2D element count");
        failures += !check(
            tensor.strides() == std::vector<std::size_t>{3, 1},
            "2D row-major strides"
        );
    }

    // 4D batch of images: NCHW.
    {
        pinn::Tensor tensor({2, 3, 4, 5});

        failures += !check(tensor.rank() == 4, "4D rank");
        failures += !check(tensor.size() == 120, "4D element count");
        failures += !check(
            tensor.strides() ==
                std::vector<std::size_t>{60, 20, 5, 1},
            "4D NCHW strides"
        );
    }

    // Data access.
    {
        pinn::Tensor tensor({2, 2});

        tensor[0] = 1.0f;
        tensor[1] = 2.0f;
        tensor[2] = 3.0f;
        tensor[3] = 4.0f;

        failures += !check(
            tensor.data()[2] == 3.0f,
            "Contiguous data access"
        );
    }

    // Data type and layout.
    {
        pinn::Tensor tensor({2, 2});

        failures += !check(
            tensor.dtype() == pinn::DataType::Float32,
            "Float32 data type"
        );

        failures += !check(
            tensor.is_contiguous(),
            "Contiguous layout"
        );
    }

    // Invalid empty shape.
    {
        bool caught = false;

        try {
            pinn::Tensor tensor({});
        } catch (const std::invalid_argument&) {
            caught = true;
        }

        failures += !check(caught, "Empty shape rejection");
    }

    // Invalid zero dimension.
    {
        bool caught = false;

        try {
            pinn::Tensor tensor({2, 0, 3});
        } catch (const std::invalid_argument&) {
            caught = true;
        }

        failures += !check(caught, "Zero dimension rejection");
    }

    // Overflow protection.
    {
        bool caught = false;

        try {
            pinn::Tensor tensor({
                static_cast<std::size_t>(-1),
                2
            });
        } catch (const std::overflow_error&) {
            caught = true;
        }

        failures += !check(caught, "Dimension overflow rejection");
    }

    std::cout << "\nTests passed: "
              << 12 - failures << "/12\n";

    return failures == 0 ? 0 : 1;
}
