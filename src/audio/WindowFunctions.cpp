#include "audio/WindowFunctions.hpp"
#include "core/Errors.hpp"

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::vector<double> WindowFunctions::generate(Type type, size_t size) {
    if (size == 0) {
        throw dissonance::DspError("Window size must be greater than 0");
    }

    switch (type) {
    case Type::Hann:
        return generateHann(size);
    case Type::Hamming:
        return generateHamming(size);
    default:
        throw dissonance::DspError("Unknown window type");
    }
}

std::vector<double> WindowFunctions::generateHann(size_t size) {
    std::vector<double> window(size);

    if (size == 1) {
        window[0] = 1.0;
        return window;
    }

    for (size_t n = 0; n < size; ++n) {
        // Hann window: w(n) = 0.5 * (1 - cos(2πn/(N-1)))
        window[n] =
            0.5 *
            (1.0 - std::cos(2.0 * M_PI * static_cast<double>(n) / static_cast<double>(size - 1)));
    }

    return window;
}

std::vector<double> WindowFunctions::generateHamming(size_t size) {
    std::vector<double> window(size);

    if (size == 1) {
        window[0] = 1.0;
        return window;
    }

    for (size_t n = 0; n < size; ++n) {
        // Hamming window: w(n) = 0.54 - 0.46 * cos(2πn/(N-1))
        window[n] = 0.54 - 0.46 * std::cos(2.0 * M_PI * static_cast<double>(n) /
                                           static_cast<double>(size - 1));
    }

    return window;
}

void WindowFunctions::apply(std::vector<double> &samples, const std::vector<double> &window) {
    if (samples.size() != window.size()) {
        throw dissonance::DspError("Sample buffer size must match window size");
    }

    for (size_t i = 0; i < samples.size(); ++i) {
        samples[i] *= window[i];
    }
}

void WindowFunctions::apply(std::vector<int16_t> &samples, const std::vector<double> &window) {
    if (samples.size() != window.size()) {
        throw dissonance::DspError("Sample buffer size must match window size");
    }

    constexpr int minVal = std::numeric_limits<int16_t>::min();
    constexpr int maxVal = std::numeric_limits<int16_t>::max();

    for (size_t i = 0; i < samples.size(); ++i) {
        const double windowed = static_cast<double>(samples[i]) * window[i];
        const int rounded = static_cast<int>(std::lround(windowed));
        samples[i] = static_cast<int16_t>(std::clamp(rounded, minVal, maxVal));
    }
}
