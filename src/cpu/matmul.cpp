#include <cstddef>
#include <stdexcept>

namespace pinn::cpu {

void matmul(
    const float* A,
    const float* B,
    float* C,
    std::size_t M,
    std::size_t N,
    std::size_t K
) {
    if (A == nullptr || B == nullptr || C == nullptr) {
        throw std::invalid_argument("matmul: null pointer");
    }

    // C[M x N] = A[M x K] * B[K x N]
    for (std::size_t i = 0; i < M; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
            float sum = 0.0f;

            for (std::size_t k = 0; k < K; ++k) {
                sum += A[i * K + k] * B[k * N + j];
            }

            C[i * N + j] = sum;
        }
    }
}

} // namespace pinn::cpu
