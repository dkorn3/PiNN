#pragma once

#include <cstddef>

namespace pinn::gpu {

void matmul(
    const float* A,
    const float* B,
    float* C,
    std::size_t M,
    std::size_t N,
    std::size_t K
);

}

