#include <pinn/gpu/matmul.h>
#include <V3DLib.h>
#include "Kernels/Matrix.h"

#include <cstddef>
#include <limits>
#include <stdexcept>

namespace pinn::gpu {

namespace {

std::size_t pad16(std::size_t value) {
    constexpr std::size_t max =
        static_cast<std::size_t>(std::numeric_limits<int>::max());

    if (value == 0) {
        throw std::invalid_argument("matmul: dimensions must be positive");
    }

    if (value > max - 15) {
        throw std::overflow_error("matmul: dimension too large");
    }

    return ((value + 15) / 16) * 16;
}

void check_size(std::size_t rows, std::size_t columns) {
    const std::size_t max =
        static_cast<std::size_t>(std::numeric_limits<int>::max());

    if (rows > max / columns) {
        throw std::overflow_error("matmul: matrix allocation too large");
    }
}

} // namespace

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

    // Pad dimensions for V3DLib's 16-element alignment requirement.
    const std::size_t Mp = pad16(M);
    const std::size_t Np = pad16(N);
    const std::size_t Kp = pad16(K);

    check_size(Mp, Kp);
    check_size(Np, Kp);
    check_size(Mp, Np);

    using namespace V3DLib;

    // V3DLib expects B to be transposed.
    Float::Array2D a(
        static_cast<int>(Mp), static_cast<int>(Kp)
    );

    Float::Array2D b(
        static_cast<int>(Np), static_cast<int>(Kp)
    );

    Float::Array2D result(
        static_cast<int>(Mp), static_cast<int>(Np)
    );

    a.fill(0.0f);
    b.fill(0.0f);
    result.fill(0.0f);

    // Copy A into the padded buffer.
    for (std::size_t r = 0; r < M; ++r) {
        for (std::size_t k = 0; k < K; ++k) {
            a[static_cast<int>(r)][static_cast<int>(k)] =
                A[r * K + k];
        }
    }

    // Copy B transposed into the padded buffer.
    for (std::size_t k = 0; k < K; ++k) {
        for (std::size_t c = 0; c < N; ++c) {
            b[static_cast<int>(c)][static_cast<int>(k)] =
                B[k * N + c];
        }
    }

    // Configure and compile the V3DLib matrix kernel.
    auto kernel = compile(
        kernels::matrix_mult_decorator(
            static_cast<int>(Mp),
            static_cast<int>(Kp),
            static_cast<int>(Np)
        )
    );

    kernel.load(&result, &a, &b).run();

    // Copy the actual M x N result back to the caller.
    for (std::size_t r = 0; r < M; ++r) {
        for (std::size_t c = 0; c < N; ++c) {
            C[r * N + c] =
                result[static_cast<int>(r)][static_cast<int>(c)];
        }
    }
}

} // namespace pinn::gpu
