/**
 * @file GainStage.cpp
 * @brief Linear amplitude gain stage with [-1, 1] output clamping.
 */

#include "audio/GainStage.hpp"

#include <algorithm>

GainStage::GainStage(double gain) : gain_(static_cast<float>(gain)) {}

void GainStage::process(std::vector<float> &samples, uint16_t /*numChannels*/) {
    if (gain_ <= 0.0f) {
        std::fill(samples.begin(), samples.end(), 0.0f);
        return;
    }
    for (auto &s : samples)
        s = std::clamp(s * gain_, -1.0f, 1.0f);
}
