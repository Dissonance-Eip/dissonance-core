/**
 * @file WindowFunctions.cpp
 * @brief Hann and Hamming window generation and application.
 */

#include "audio/WindowFunctions.hpp"
#include "core/Errors.hpp"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace window {

namespace {

std::vector<float> generateHann(size_t size) {
    std::vector<float> w(size);
    if (size == 1) {
        w[0] = 1.0f;
        return w;
    }
    for (size_t n = 0; n < size; ++n)
        w[n] = 0.5f * (1.0f - std::cos(2.0f * static_cast<float>(M_PI) *
                                        static_cast<float>(n) / static_cast<float>(size - 1)));
    return w;
}

std::vector<float> generateHamming(size_t size) {
    std::vector<float> w(size);
    if (size == 1) {
        w[0] = 1.0f;
        return w;
    }
    for (size_t n = 0; n < size; ++n)
        w[n] = 0.54f - 0.46f * std::cos(2.0f * static_cast<float>(M_PI) *
                                         static_cast<float>(n) / static_cast<float>(size - 1));
    return w;
}

} // namespace

std::vector<float> generate(Type type, size_t size) {
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

void apply(std::vector<float> &samples, const std::vector<float> &coefficients) {
    if (samples.size() != coefficients.size())
        throw dissonance::DspError("Sample buffer size must match window size");
    for (size_t i = 0; i < samples.size(); ++i)
        samples[i] *= coefficients[i];
}

} // namespace window
