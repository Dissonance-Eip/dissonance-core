#pragma once

#include "audio/AudioStage.hpp"

class GainStage : public AudioStage {
  public:
    explicit GainStage(double gain);
    void process(std::vector<float> &samples, uint16_t numChannels) override;

  private:
    double gain_;
};
