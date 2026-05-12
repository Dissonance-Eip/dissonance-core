#pragma once

#include <cstdint>
#include <vector>

class AudioStage {
  public:
    virtual ~AudioStage() = default;
    virtual void process(std::vector<float> &samples, uint16_t numChannels) = 0;
};
