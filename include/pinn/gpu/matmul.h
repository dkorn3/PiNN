#pragma once

#include <cstddef>
#include <memory>

namespace pinn::gpu {

class Matmul {
public:
    Matmul(
        std::size_t M,
        std::size_t N,
        std::size_t K
    );

    ~Matmul();

    Matmul(Matmul&&) noexcept;
    Matmul& operator=(Matmul&&) noexcept;

    Matmul(const Matmul&) = delete;
    Matmul& operator=(const Matmul&) = delete;

    void compute(
        const float* A,
        const float* B,
        float* C
    );

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};


} // namespace pinn::gpu
