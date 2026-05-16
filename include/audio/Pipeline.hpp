#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "audio/AudioStage.hpp"

class Pipeline {
  public:
    void addStage(std::unique_ptr<AudioStage> stage);
    void run(std::vector<float> &samples, uint16_t numChannels);

  private:
    std::vector<std::unique_ptr<AudioStage>> stages_;
};
