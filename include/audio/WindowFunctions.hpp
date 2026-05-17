#pragma once

#include <cstddef>
#include <vector>

/**
 * @brief Window function generation and application for spectral analysis.
 *
 * Window functions taper a signal frame to zero (or near-zero) at its edges,
 * reducing spectral leakage before an FFT is applied.
 */
namespace window {

/** @brief Supported window shapes. */
enum class Type {
    Hann,   ///< Hann window — tapers fully to 0 at both edges.
    Hamming ///< Hamming window — tapers to ~0.08 at edges, lower sidelobe roll-off.
};

/**
 * @brief Generate a window of the given type and size.
 * @param type  Window shape.
 * @param size  Number of coefficients (must be > 0).
 * @return      Float coefficients in [0, 1].
 */
std::vector<float> generate(Type type, size_t size);

/**
 * @brief Multiply samples element-wise by the window coefficients in-place.
 * @param samples       Sample buffer to window. Must be the same length as coefficients.
 * @param coefficients  Window coefficients produced by generate().
 */
void apply(std::vector<float> &samples, const std::vector<float> &coefficients);

} // namespace window
