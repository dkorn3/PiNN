#include <pinn/tensor.h>

#include <limits>
#include <stdexcept>
#include <utility>

namespace pinn {

Tensor::Tensor(std::vector<std::size_t> shape)
    : shape_(std::move(shape))
{
    if (shape_.empty()) {
        throw std::invalid_argument(
            "Tensor: shape must contain at least one dimension"
        );
    }

    std::size_t elements = 1;

    for (const std::size_t dimension : shape_) {
        if (dimension == 0) {
            throw std::invalid_argument(
                "Tensor: dimensions must be positive"
            );
        }

        if (elements >
            std::numeric_limits<std::size_t>::max() / dimension) {
            throw std::overflow_error(
                "Tensor: element count overflow"
            );
        }

        elements *= dimension;
    }

    data_.resize(elements);
    calculate_strides();
}

void Tensor::calculate_strides() {
    strides_.resize(shape_.size());

    std::size_t stride = 1;

    for (std::size_t i = shape_.size(); i-- > 0;) {
        strides_[i] = stride;
        stride *= shape_[i];
    }
}

const std::vector<std::size_t>& Tensor::shape() const noexcept {
    return shape_;
}

const std::vector<std::size_t>& Tensor::strides() const noexcept {
    return strides_;
}

std::size_t Tensor::rank() const noexcept {
    return shape_.size();
}

std::size_t Tensor::size() const noexcept {
    return data_.size();
}

DataType Tensor::dtype() const noexcept {
    return DataType::Float32;
}

bool Tensor::is_contiguous() const noexcept {
    return true;
}

float* Tensor::data() noexcept {
    return data_.data();
}

const float* Tensor::data() const noexcept {
    return data_.data();
}

float& Tensor::operator[](std::size_t index) {
    return data_.at(index);
}

const float& Tensor::operator[](std::size_t index) const {
    return data_.at(index);
}

} // namespace pinn
