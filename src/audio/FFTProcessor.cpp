#include "audio/FFTProcessor.hpp"
#include "core/Errors.hpp"

#include <algorithm>
#include <cmath>

namespace fft {

std::vector<std::complex<double>> transform(const std::vector<double> &input) {
    if (input.empty())
        throw dissonance::DspError("FFT input cannot be empty");

    const size_t N = input.size();
    std::vector<std::complex<double>> output(N);

    const double twoPiOverN = 2.0 * std::acos(-1.0) / static_cast<double>(N);
    for (size_t k = 0; k < N; ++k) {
        std::complex<double> sum{0.0, 0.0};
        for (size_t n = 0; n < N; ++n) {
            const double angle = twoPiOverN * static_cast<double>(k * n);
            sum += input[n] * std::complex<double>{std::cos(angle), -std::sin(angle)};
        }
        output[k] = sum;
    }
    return output;
}

std::vector<double> inverse(const std::vector<std::complex<double>> &spectrum) {
    if (spectrum.empty())
        throw dissonance::DspError("iFFT input cannot be empty");

    const size_t N = spectrum.size();
    std::vector<double> output(N);

    const double twoPiOverN = 2.0 * std::acos(-1.0) / static_cast<double>(N);
    for (size_t n = 0; n < N; ++n) {
        std::complex<double> sum{0.0, 0.0};
        for (size_t k = 0; k < N; ++k) {
            const double angle = twoPiOverN * static_cast<double>(k * n);
            sum += spectrum[k] * std::complex<double>{std::cos(angle), std::sin(angle)};
        }
        output[n] = sum.real() / static_cast<double>(N);
    }
    return output;
}

std::vector<double> magnitude(const std::vector<std::complex<double>> &spectrum) {
    std::vector<double> mags(spectrum.size());
    std::transform(spectrum.begin(), spectrum.end(), mags.begin(),
                   [](const std::complex<double> &v) { return std::abs(v); });
    return mags;
}

} // namespace fft
