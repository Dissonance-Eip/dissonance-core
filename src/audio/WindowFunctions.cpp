#include "audio/WindowFunctions.hpp"
#include "core/Errors.hpp"

#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace window {

namespace {

std::vector<double> generateHann(size_t size) {
    std::vector<double> w(size);
    if (size == 1) {
        w[0] = 1.0;
        return w;
    }
    for (size_t n = 0; n < size; ++n)
        w[n] =
            0.5 *
            (1.0 - std::cos(2.0 * M_PI * static_cast<double>(n) / static_cast<double>(size - 1)));
    return w;
}

std::vector<double> generateHamming(size_t size) {
    std::vector<double> w(size);
    if (size == 1) {
        w[0] = 1.0;
        return w;
    }
    for (size_t n = 0; n < size; ++n)
        w[n] = 0.54 -
               0.46 * std::cos(2.0 * M_PI * static_cast<double>(n) / static_cast<double>(size - 1));
    return w;
}

} // namespace

std::vector<double> generate(Type type, size_t size) {
    if (size == 0)
        throw dissonance::DspError("Window size must be greater than 0");
    switch (type) {
    case Type::Hann:
        return generateHann(size);
    case Type::Hamming:
        return generateHamming(size);
    default:
        throw dissonance::DspError("Unknown window type");
    }
}

void apply(std::vector<float> &samples, const std::vector<double> &coefficients) {
    if (samples.size() != coefficients.size())
        throw dissonance::DspError("Sample buffer size must match window size");
    for (size_t i = 0; i < samples.size(); ++i)
        samples[i] *= static_cast<float>(coefficients[i]);
}

} // namespace window
