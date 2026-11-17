#include <pinn/ops/activation.h>

#include <algorithm>

namespace pinn::ops {

void relu(Tensor& input) {
    float* values = input.data();

    std::transform(
        values,
        values + input.size(),
        values,
        [](float value) {
            return std::max(0.0f, value);
        }
    );
}

} // namespace pinn::ops
