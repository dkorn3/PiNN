#pragma once

#include <cstddef>
#include <vector>

namespace pinn {

enum class DataType {
    Float32
};

class Tensor {
public:
    explicit Tensor(std::vector<std::size_t> shape);

    const std::vector<std::size_t>& shape() const noexcept;
    const std::vector<std::size_t>& strides() const noexcept;

    std::size_t rank() const noexcept;
    std::size_t size() const noexcept;

    DataType dtype() const noexcept;
    bool is_contiguous() const noexcept;

    float* data() noexcept;
    const float* data() const noexcept;

    float& operator[](std::size_t index);
    const float& operator[](std::size_t index) const;

private:
    std::vector<std::size_t> shape_;
    std::vector<std::size_t> strides_;
    std::vector<float> data_;

    void calculate_strides();
};

} // namespace pinn
