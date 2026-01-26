#include "FFTProcessor.hpp"

#include <cmath>
#include <stdexcept>

std::vector<std::complex<double>> FFTProcessor::fft(const std::vector<double>& input) {
    if (input.empty()) {
        throw std::invalid_argument("FFT input cannot be empty");
    }

    const size_t N = input.size();
    std::vector<std::complex<double>> output(N);

    const double pi = std::acos(-1.0);
    const double twoPiOverN = 2.0 * pi / static_cast<double>(N);

    for (size_t k = 0; k < N; ++k) {
        std::complex<double> sum{0.0, 0.0};
        for (size_t n = 0; n < N; ++n) {
            const double angle = twoPiOverN * static_cast<double>(k * n);
            const std::complex<double> expTerm{std::cos(angle), -std::sin(angle)};
            sum += input[n] * expTerm;
        }
        output[k] = sum;
    }

    return output;
}

std::vector<double> FFTProcessor::ifft(const std::vector<std::complex<double>>& spectrum) {
    if (spectrum.empty()) {
        throw std::invalid_argument("iFFT input cannot be empty");
    }

    const size_t N = spectrum.size();
    std::vector<double> output(N);

    const double pi = std::acos(-1.0);
    const double twoPiOverN = 2.0 * pi / static_cast<double>(N);

    for (size_t n = 0; n < N; ++n) {
        std::complex<double> sum{0.0, 0.0};
        for (size_t k = 0; k < N; ++k) {
            const double angle = twoPiOverN * static_cast<double>(k * n);
            const std::complex<double> expTerm{std::cos(angle), std::sin(angle)};
            sum += spectrum[k] * expTerm;
        }
        output[n] = sum.real() / static_cast<double>(N);
    }

    return output;
}

std::vector<double> FFTProcessor::magnitude(const std::vector<std::complex<double>>& spectrum) {
    std::vector<double> mags;
    mags.reserve(spectrum.size());
    for (const auto& c : spectrum) {
        mags.push_back(std::abs(c));
    }
    return mags;
}
