#include <pinn/gpu/matmul.h>

#include <cstddef>
#include <iostream>
#include <stdexcept>

int main() {
    constexpr std::size_t M = 16;
    constexpr std::size_t N = 16;
    constexpr std::size_t K = 16;

    float A[M * K];
    float B[K * N];
    float C[M * N] = {};

    for (std::size_t i = 0; i < M * K; ++i) {
        A[i] = 1.0f;
    }

    for (std::size_t i = 0; i < K * N; ++i) {
        B[i] = 1.0f;
    }

    try {
        pinn::gpu::matmul(A, B, C, M, N, K);
    } catch (const std::exception& e) {
        std::cerr << "GPU matmul failed: " << e.what() << '\n';
        return 1;
    }

    for (std::size_t i = 0; i < M * N; ++i) {
        if (C[i] != 16.0f) {
            std::cerr << "Incorrect result at index " << i
                      << ": " << C[i] << '\n';
            return 1;
        }
    }

    std::cout << "GPU matrix multiplication passed.\n";
    std::cout << "Result: " << C[0] << '\n';

    return 0;
}
