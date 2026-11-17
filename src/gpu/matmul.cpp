
#include <pinn/gpu/matmul.h>

#include <V3DLib.h>
#include "Kernels/Matrix.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

namespace pinn::gpu {

namespace {

using Clock = std::chrono::steady_clock;

double elapsed_us(Clock::time_point start, Clock::time_point end) {
    return std::chrono::duration<double, std::micro>(
        end - start
    ).count();
}

bool profiling_enabled() {
    const char* value = std::getenv("PINN_PROFILE");
    return value != nullptr && value[0] == '1';
}

std::size_t pad16(std::size_t value) {
    constexpr std::size_t max =
        static_cast<std::size_t>(std::numeric_limits<int>::max());

    if (value == 0) {
        throw std::invalid_argument(
            "matmul: dimensions must be positive"
        );
    }

    if (value > max - 15) {
        throw std::overflow_error(
            "matmul: dimension too large"
        );
    }

    return ((value + 15) / 16) * 16;
}

void check_size(std::size_t rows, std::size_t columns) {
    const std::size_t max =
        static_cast<std::size_t>(std::numeric_limits<int>::max());

    if (rows > max / columns) {
        throw std::overflow_error(
            "matmul: matrix allocation too large"
        );
    }
}

struct MatrixShape {
    std::size_t M;
    std::size_t N;
    std::size_t K;
    std::size_t Mp;
    std::size_t Np;
    std::size_t Kp;
};

MatrixShape make_shape(
    std::size_t M,
    std::size_t N,
    std::size_t K
) {
    MatrixShape shape{
        M, N, K,
        pad16(M),
        pad16(N),
        pad16(K)
    };

    check_size(shape.Mp, shape.Kp);
    check_size(shape.Np, shape.Kp);
    check_size(shape.Mp, shape.Np);

    return shape;
}

} // namespace

struct Matmul::Impl {
    std::size_t M;
    std::size_t N;
    std::size_t K;

    std::size_t Mp;
    std::size_t Np;
    std::size_t Kp;

    V3DLib::Float::Array2D a;
    V3DLib::Float::Array2D b;
    V3DLib::Float::Array2D result;

    std::unique_ptr<V3DLib::BaseKernel> kernel;

    explicit Impl(const MatrixShape& shape)
        : M(shape.M),
          N(shape.N),
          K(shape.K),
          Mp(shape.Mp),
          Np(shape.Np),
          Kp(shape.Kp),
          a(static_cast<int>(Mp), static_cast<int>(Kp)),
          b(static_cast<int>(Np), static_cast<int>(Kp)),
          result(static_cast<int>(Mp), static_cast<int>(Np)),
          kernel(
              std::make_unique<V3DLib::BaseKernel>(
                  V3DLib::compile(
                      kernels::matrix_mult_decorator(
                          static_cast<int>(Mp),
                          static_cast<int>(Kp),
                          static_cast<int>(Np)
                      )
                  )
              )
          )
    {
        // Use the maximum QPU count supported by the detected hardware.
        kernel->setMaxQPUs();

        std::cerr << "PiNN GPU QPUs: "
                  << kernel->numQPUs() << '\n';

        a.fill(0.0f);
        b.fill(0.0f);
        result.fill(0.0f);
    }

    Impl(
        std::size_t rows,
        std::size_t columns,
        std::size_t inner
    )
        : Impl(make_shape(rows, columns, inner))
    {}

    void execute(
        const float* A,
        const float* B,
        float* C
    ) {
        if (A == nullptr || B == nullptr || C == nullptr) {
            throw std::invalid_argument(
                "matmul: null pointer"
            );
        }

        const bool profile = profiling_enabled();
        const auto total_start = Clock::now();

        float* a_data = a.ptr();
        float* b_data = b.ptr();
        float* result_data = result.ptr();

        // Stage 1: Pack A.
        const auto a_start = Clock::now();

        for (std::size_t r = 0; r < M; ++r) {
            std::copy_n(
                A + r * K,
                K,
                a_data + r * Kp
            );
        }

        const auto a_end = Clock::now();

        // Stage 2: Transpose B.
        const auto b_start = Clock::now();

        for (std::size_t k = 0; k < K; ++k) {
            for (std::size_t c = 0; c < N; ++c) {
                b_data[c * Kp + k] = B[k * N + c];
            }
        }

        const auto b_end = Clock::now();

        // Stage 3: Load kernel arguments, dispatch, execute and wait.
        const auto kernel_start = Clock::now();

        kernel->load(&result, &a, &b).run();

        const auto kernel_end = Clock::now();

        // Stage 4: Copy result back to the caller.
        const auto output_start = Clock::now();

        for (std::size_t r = 0; r < M; ++r) {
            std::copy_n(
                result_data + r * Np,
                N,
                C + r * N
            );
        }

        const auto output_end = Clock::now();

        if (profile) {
            std::cerr
                << std::fixed << std::setprecision(2)
                << "[PiNN profile " << M << "x" << N << "x" << K << "] "
                << "A pack: " << elapsed_us(a_start, a_end) << " us | "
                << "B transpose: " << elapsed_us(b_start, b_end) << " us | "
                << "Kernel load/run: " << elapsed_us(kernel_start, kernel_end) << " us | "
                << "Output: " << elapsed_us(output_start, output_end) << " us | "
                << "Total: " << elapsed_us(total_start, output_end) << " us\n";
        }
    }
};

Matmul::Matmul(
    std::size_t M,
    std::size_t N,
    std::size_t K
)
    : impl_(std::make_unique<Impl>(M, N, K))
{}

Matmul::~Matmul() = default;

Matmul::Matmul(Matmul&&) noexcept = default;

Matmul& Matmul::operator=(Matmul&&) noexcept = default;

void Matmul::compute(
    const float* A,
    const float* B,
    float* C
) {
    if (!impl_) {
        throw std::logic_error(
            "matmul: context has been moved from"
        );
    }

    impl_->execute(A, B, C);
}


} // namespace pinn::gpu
