#include <cstddef>
#include <iostream>

namespace pinn::cpu {
void matmul(const float* A, const float* B, float* C,
            std::size_t M, std::size_t N, std::size_t K);
}

int main() {
    constexpr std::size_t M = 2;
    constexpr std::size_t N = 2;
    constexpr std::size_t K = 3;

    const float A[M * K] = {
        1, 2, 3,
        4, 5, 6
    };

    const float B[K * N] = {
        7, 8,
        9, 10,
        11, 12
    };

    float C[M * N] = {};

    pinn::cpu::matmul(A, B, C, M, N, K);

    std::cout << "PiNN CPU Matrix Multiplication\n";
    std::cout << "Result:\n";

    for (std::size_t i = 0; i < M; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
            std::cout << C[i * N + j] << " ";
        }
        std::cout << '\n';
    }

    return 0;
}
