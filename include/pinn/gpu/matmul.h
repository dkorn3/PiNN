#pragma once

#include <cstddef>
#include <memory>

namespace pinn::gpu {

class MatmulContext {
public:
    MatmulContext(
        std::size_t M,
        std::size_t N,
        std::size_t K
    );

    ~MatmulContext();

    MatmulContext(MatmulContext&&) noexcept;
    MatmulContext& operator=(MatmulContext&&) noexcept;

    MatmulContext(const MatmulContext&) = delete;
    MatmulContext& operator=(const MatmulContext&) = delete;

    void matmul(
        const float* A,
        const float* B,
        float* C
    );

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// One-shot GPU matrix multiplication.
void matmul(
    const float* A,
    const float* B,
    float* C,
    std::size_t M,
    std::size_t N,
    std::size_t K
);

} // namespace pinn::gpu
