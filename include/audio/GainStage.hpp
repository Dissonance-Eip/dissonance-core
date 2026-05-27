#pragma once

#include "audio/AudioStage.hpp"

/**
 * @brief AudioStage that scales every sample by a fixed gain factor.
 *
 * Output is clamped to [-1, 1]. A gain of 0 silences the signal.
 */
class GainStage : public AudioStage {
  public:
    /** @param gain  Linear amplitude multiplier (e.g. 0.8 = -2 dBFS). */
    explicit GainStage(double gain);
    void process(std::vector<float> &samples, uint16_t numChannels) override;

  private:
    float gain_;
};
