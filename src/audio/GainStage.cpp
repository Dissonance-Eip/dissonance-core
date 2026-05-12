#include "audio/GainStage.hpp"
#include "audio/GainProcessor.hpp"

GainStage::GainStage(double gain) : gain_(gain) {}

void GainStage::process(std::vector<float> &samples, uint16_t /*numChannels*/) {
    GainProcessor(gain_).apply(samples);
}
