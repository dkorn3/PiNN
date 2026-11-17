#include <pinn/gpu/matmul.h>

#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

bool test_case(
    const char* name,
    std::size_t M,
    std::size_t N,
    std::size_t K,
    const std::vector<float>& A,
    const std::vector<float>& B,
    const std::vector<float>& expected
) {
    pinn::gpu::Matmul matmul(M, N, K);

    std::vector<float> output(M * N, -999.0f);
    matmul.compute(A.data(), B.data(), output.data());

    for (std::size_t i = 0; i < output.size(); ++i) {
        if (std::fabs(output[i] - expected[i]) > 1e-4f) {
            std::cerr << "FAIL: " << name
                      << ", index " << i
                      << ", expected " << expected[i]
                      << ", got " << output[i] << '\n';
            return false;
        }
    }

    std::cout << "PASS: " << name << '\n';
    return true;
}

bool test_null_pointer() {
    pinn::gpu::Matmul matmul(2, 2, 2);

    const std::vector<float> B = {1, 2, 3, 4};
    std::vector<float> C(4);

    try {
        matmul.compute(nullptr, B.data(), C.data());
    } catch (const std::invalid_argument&) {
        std::cout << "PASS: null pointer rejection\n";
        return true;
    }

    std::cerr << "FAIL: null pointer was not rejected\n";
    return false;
}

bool test_moved_from_object() {
    pinn::gpu::Matmul original(2, 2, 2);
    pinn::gpu::Matmul moved(std::move(original));

    const std::vector<float> A = {1, 2, 3, 4};
    const std::vector<float> B = {1, 0, 0, 1};
    std::vector<float> C(4);

    try {
        original.compute(A.data(), B.data(), C.data());
    } catch (const std::logic_error&) {
        std::cout << "PASS: moved-from object rejection\n";
        return true;
    }

    std::cerr << "FAIL: moved-from object was not rejected\n";
    return false;
}

int main() {
    int failures = 0;
    int tests = 0;

    ++tests;
    failures += !test_case(
        "rectangular multiplication",
        2, 2, 3,
        {1, 2, 3, 4, 5, 6},
        {7, 8, 9, 10, 11, 12},
        {58, 64, 139, 154}
    );

    ++tests;
    failures += !test_case(
        "non-multiple-of-16 dimensions",
        3, 5, 3,
        {
            1, 2, 3,
            4, 5, 6,
            7, 8, 9
        },
        {
            1, 2, 3, 4, 5,
            6, 7, 8, 9, 10,
            11, 12, 13, 14, 15
        },
        {
            46, 52, 58, 64, 70,
            100, 115, 130, 145, 160,
            154, 178, 202, 226, 250
        }
    );

    ++tests;
    failures += !test_case(
        "identity multiplication",
        3, 3, 3,
        {
            1, 2, 3,
            4, 5, 6,
            7, 8, 9
        },
        {
            1, 0, 0,
            0, 1, 0,
            0, 0, 1
        },
        {
            1, 2, 3,
            4, 5, 6,
            7, 8, 9
        }
    );

    ++tests;
    failures += !test_null_pointer();

    ++tests;
    failures += !test_moved_from_object();

    std::cout << "\nTests passed: "
              << tests - failures << "/" << tests << '\n';

    return failures == 0 ? 0 : 1;
}
