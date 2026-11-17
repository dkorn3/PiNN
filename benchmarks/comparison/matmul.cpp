#include "pinn/cpu/matmul.h"
#include "pinn/gpu/matmul.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;

constexpr int WARMUP_RUNS = 1;
constexpr int BENCHMARK_RUNS = 3;

double benchmark_cpu(
    const std::vector<float>& A,
    const std::vector<float>& B,
    std::vector<float>& C,
    std::size_t N
) {
    const auto start = Clock::now();

    for (int i = 0; i < BENCHMARK_RUNS; ++i) {
        pinn::cpu::matmul(A.data(), B.data(), C.data(), N, N, N);
    }

    const auto end = Clock::now();

    return std::chrono::duration<double, std::micro>(
        end - start
    ).count() / BENCHMARK_RUNS;
}

double benchmark_gpu(
    pinn::gpu::Matmul& context,
    const std::vector<float>& A,
    const std::vector<float>& B,
    std::vector<float>& C
) {
    const auto start = Clock::now();

    for (int i = 0; i < BENCHMARK_RUNS; ++i) {
        context.compute(A.data(), B.data(), C.data());
    }

    const auto end = Clock::now();

    return std::chrono::duration<double, std::micro>(
        end - start
    ).count() / BENCHMARK_RUNS;
}

bool verify_results(
    const std::vector<float>& reference,
    const std::vector<float>& result,
    float& max_error
) {
    max_error = 0.0f;

    for (std::size_t i = 0; i < reference.size(); ++i) {
        const float error = std::fabs(reference[i] - result[i]);
        max_error = std::max(max_error, error);

        if (error > 1e-3f) {
            return false;
        }
    }

    return true;
}

int main() {
    const std::vector<std::size_t> sizes = {
        16, 32, 64, 128
    };

    std::mt19937 generator(42);
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

    std::cout << "PiNN CPU vs. VideoCore IV QPU Benchmark\n";
    std::cout << "========================================\n";
    std::cout << "Warm-up runs: " << WARMUP_RUNS << '\n';
    std::cout << "Measured runs: " << BENCHMARK_RUNS << '\n';
    std::cout << "GPU uses a reusable Matmul.\n";
    std::cout << "Context setup and kernel compilation are excluded.\n\n";

    std::cout << std::left
              << std::setw(8) << "Size"
              << std::right
              << std::setw(14) << "CPU (us)"
              << std::setw(14) << "GPU (us)"
              << std::setw(14) << "Speedup"
              << std::setw(14) << "Max Error"
              << '\n';

    std::cout << std::string(64, '-') << '\n';

    try {
        for (const std::size_t N : sizes) {
            const std::size_t elements = N * N;

            std::vector<float> A(elements);
            std::vector<float> B(elements);
            std::vector<float> cpu_result(elements, 0.0f);
            std::vector<float> gpu_result(elements, 0.0f);

            for (std::size_t i = 0; i < elements; ++i) {
                A[i] = distribution(generator);
                B[i] = distribution(generator);
            }

            // Create the reusable GPU context outside the timed region.
            pinn::gpu::Matmul gpu_context(N, N, N);

            // Warm up both implementations.
            for (int i = 0; i < WARMUP_RUNS; ++i) {
                pinn::cpu::matmul(
                    A.data(), B.data(), cpu_result.data(), N, N, N
                );

                gpu_context.compute(
                    A.data(), B.data(), gpu_result.data()
                );
            }

            const double cpu_time = benchmark_cpu(
                A, B, cpu_result, N
            );

            const double gpu_time = benchmark_gpu(
                gpu_context, A, B, gpu_result
            );

            float max_error = 0.0f;

            if (!verify_results(cpu_result, gpu_result, max_error)) {
                std::cerr << "Correctness check failed for "
                          << N << "x" << N << " matrices.\n";
                std::cerr << "Maximum error: "
                          << max_error << '\n';
                return 1;
            }

            const double speedup = cpu_time / gpu_time;

            std::cout << std::left << std::setw(8) << N
                      << std::right
                      << std::fixed << std::setprecision(2)
                      << std::setw(14) << cpu_time
                      << std::setw(14) << gpu_time
                      << std::setw(13) << speedup << "x"
                      << std::scientific << std::setprecision(3)
                      << std::setw(14) << max_error
                      << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << "\nBenchmark failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "\nAll CPU/GPU correctness checks passed.\n";

    return 0;
}
