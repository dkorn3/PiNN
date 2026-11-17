#include <pinn/gpu/relu.h>

#include <V3DLib.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>

namespace pinn::gpu {

namespace {

void relu_kernel(
    V3DLib::Float::Ptr output,
    V3DLib::Float::Ptr input,
    V3DLib::Int blocks
) {
    using namespace V3DLib;

    V3DLib::Int offset = 16 * numQPUs();

    output += 16 * me();
    input += 16 * me();

    For (V3DLib::Int block = me(),
         block < blocks,
         block += numQPUs())

        V3DLib::Float value = *input;
        *output = V3DLib::max(value, 0.0f);

        output += offset;
        input += offset;
    End
    using V3DLib::End_;
}

std::size_t padded_size(std::size_t size) {
    if (size == 0) {
        throw std::invalid_argument(
            "relu: size must be positive"
        );
    }

    if (size > std::numeric_limits<std::size_t>::max() - 15) {
        throw std::overflow_error(
            "relu: size overflow"
        );
    }

    const std::size_t blocks = (size + 15) / 16;
    const std::size_t max_blocks =
        std::numeric_limits<std::uint32_t>::max() / 16;

    if (blocks > max_blocks ||
        blocks > static_cast<std::size_t>(
            std::numeric_limits<int>::max()
        )) {
        throw std::overflow_error(
            "relu: GPU allocation too large"
        );
    }

    return blocks * 16;
}

} // namespace

struct ReluContext::Impl {
    std::size_t size;
    std::size_t blocks;

    V3DLib::Float::Array input;
    V3DLib::Float::Array output;

    std::unique_ptr<V3DLib::BaseKernel> kernel;

    explicit Impl(std::size_t count)
        : size(count),
          blocks(padded_size(count) / 16),
          input(static_cast<std::uint32_t>(blocks * 16)),
          output(static_cast<std::uint32_t>(blocks * 16)),
          kernel(
              std::make_unique<V3DLib::BaseKernel>(
                  V3DLib::compile(relu_kernel)
              )
          )
    {
        kernel->setMaxQPUs();

        input.fill(0.0f);
        output.fill(0.0f);
    }

    void execute(const float* source, float* destination) {
        if (source == nullptr || destination == nullptr) {
            throw std::invalid_argument(
                "relu: null pointer"
            );
        }

        std::copy_n(source, size, input.ptr());

        kernel->load(
            &output,
            &input,
            static_cast<int>(blocks)
        ).run();

        std::copy_n(output.ptr(), size, destination);
    }
};

ReluContext::ReluContext(std::size_t size)
    : impl_(std::make_unique<Impl>(size))
{}

ReluContext::~ReluContext() = default;

ReluContext::ReluContext(ReluContext&&) noexcept = default;

ReluContext& ReluContext::operator=(ReluContext&&) noexcept = default;

void ReluContext::relu(
    const float* input,
    float* output
) {
    if (!impl_) {
        throw std::logic_error(
            "relu: context has been moved from"
        );
    }

    impl_->execute(input, output);
}

void relu(
    const float* input,
    float* output,
    std::size_t size
) {
    ReluContext context(size);
    context.relu(input, output);
}

void relu(Tensor& input) {
    relu(input.data(), input.data(), input.size());
}

} // namespace pinn::gpu
