#pragma once

#include <complex>
#include <vector>

/**
 * @brief FFT primitives backed by KissFFT.
 *
 * All functions operate on float precision, matching the audio pipeline.
 * The inverse transform applies the 1/N normalisation factor so that
 * transform → inverse is a lossless round-trip (up to float precision).
 */
namespace fft {

/** @brief Forward FFT. Returns N complex bins for N real input samples. */
std::vector<std::complex<float>> transform(const std::vector<float> &input);

/** @brief Inverse FFT with 1/N normalisation. Returns N real samples. */
std::vector<float> inverse(const std::vector<std::complex<float>> &spectrum);

/** @brief Magnitude spectrum: |bin| for each complex bin. */
std::vector<float> magnitude(const std::vector<std::complex<float>> &spectrum);

} // namespace fft
