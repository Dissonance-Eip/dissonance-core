#include "audio/GainProcessor.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

void GainProcessor::apply(std::vector<int16_t> &samples) const {
    if (gain_ <= 0.0) {
        std::fill(samples.begin(), samples.end(), static_cast<int16_t>(0));
        return;
    }

    constexpr int minVal = std::numeric_limits<int16_t>::min();
    constexpr int maxVal = std::numeric_limits<int16_t>::max();

    for (auto &sample : samples) {
        const int scaled = static_cast<int>(std::lround(static_cast<double>(sample) * gain_));
        sample = static_cast<int16_t>(std::clamp(scaled, minVal, maxVal));
    }
}
