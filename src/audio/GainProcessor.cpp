#include "audio/GainProcessor.hpp"

#include <algorithm>

void GainProcessor::apply(std::vector<float> &samples) const {
    if (gain_ <= 0.0) {
        std::fill(samples.begin(), samples.end(), 0.0f);
        return;
    }
    const float g = static_cast<float>(gain_);
    for (auto &s : samples)
        s = std::clamp(s * g, -1.0f, 1.0f);
}
