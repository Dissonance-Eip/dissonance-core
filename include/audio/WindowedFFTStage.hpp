#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include "audio/AudioStage.hpp"

class WindowedFFTStage : public AudioStage {
  public:
    WindowedFFTStage(size_t frameSize, float cutoffFraction,
                     std::function<void(float)> progressCallback = {});

    void process(std::vector<float> &samples, uint16_t numChannels) override;

    size_t framesProcessed() const { return framesProcessed_; }
    size_t bins() const { return frameSize_; }
    size_t cutoffBin() const { return cutoffBin_; }

  private:
    size_t frameSize_;
    size_t hopSize_;
    size_t cutoffBin_;
    std::vector<double> window_;
    std::vector<float> block_;
    std::function<void(float)> progressCallback_;
    size_t framesProcessed_ = 0;
};
