#include <cstddef>
#include <iostream>
#include <stdexcept>

namespace pinn::cpu {
void matmul(const float* A, const float* B, float* C,
            std::size_t M, std::size_t N, std::size_t K);
}

bool check(
    const float* actual,
    const float* expected,
    std::size_t size,
    const char* name
) {
    for (std::size_t i = 0; i < size; ++i) {
        if (actual[i] != expected[i]) {
            std::cerr << "FAIL: " << name
                      << " at index " << i
                      << ", expected " << expected[i]
                      << ", got " << actual[i] << '\n';
            return false;
        }
    }

    std::cout << "PASS: " << name << '\n';
    return true;
}

int main() {
    int failures = 0;

    // Test 1: Standard matrix multiplication.
    {
        const float A[] = {1, 2, 3, 4, 5, 6};
        const float B[] = {7, 8, 9, 10, 11, 12};
        const float expected[] = {58, 64, 139, 154};
        float C[4] = {};

        pinn::cpu::matmul(A, B, C, 2, 2, 3);

        if (!check(C, expected, 4, "Standard multiplication"))
            ++failures;
    }

    // Test 2: Rectangular output matrix.
    {
        const float A[] = {1, 2, 3, 4, 5, 6};
        const float B[] = {2, 3, 4};
        const float expected[] = {20, 47};
        float C[2] = {};

        pinn::cpu::matmul(A, B, C, 2, 1, 3);

        if (!check(C, expected, 2, "Rectangular multiplication"))
            ++failures;
    }

    // Test 3: Identity matrix.
    {
        const float A[] = {3, 5, 7, 9};
        const float I[] = {1, 0, 0, 1};
        float C[4] = {};

        pinn::cpu::matmul(A, I, C, 2, 2, 2);

        if (!check(C, A, 4, "Identity multiplication"))
            ++failures;
    }

    // Test 4: Null-pointer handling.
    {
        const float B[] = {1};
        float C[1] = {};
        bool caught = false;

        try {
            pinn::cpu::matmul(nullptr, B, C, 1, 1, 1);
        } catch (const std::invalid_argument&) {
            caught = true;
        }

        if (caught) {
            std::cout << "PASS: Null-pointer handling\n";
        } else {
            std::cerr << "FAIL: Null-pointer handling\n";
            ++failures;
        }
    }

    std::cout << "\n";
    std::cout << "Tests passed: " << 4 - failures << "/4\n";

    return failures == 0 ? 0 : 1;
}
