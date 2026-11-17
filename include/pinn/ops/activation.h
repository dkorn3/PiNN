#pragma once

#include <pinn/tensor.h>

namespace pinn::ops {

// Applies ReLU in place.
void relu(Tensor& input);

} // namespace pinn::ops
