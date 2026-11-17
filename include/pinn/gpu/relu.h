#pragma once

#include <cstddef>
#include <memory>
#include <pinn/tensor.h>
#include <pinn/tensor.h>

namespace pinn::gpu {

class ReluContext {
public:
    explicit ReluContext(std::size_t size);
    ~ReluContext();

    ReluContext(ReluContext&&) noexcept;
    ReluContext& operator=(ReluContext&&) noexcept;

    ReluContext(const ReluContext&) = delete;
    ReluContext& operator=(const ReluContext&) = delete;

    void relu(const float* input, float* output);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// One-shot GPU ReLU.
void relu(Tensor& input);

void relu(Tensor& input);

void relu(
    const float* input,
    float* output,
    std::size_t size
);

} // namespace pinn::gpu
