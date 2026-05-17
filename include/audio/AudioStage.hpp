#pragma once

#include <cstdint>
#include <vector>

/**
 * @brief Abstract base class for all audio processing stages.
 *
 * Each stage receives the full interleaved sample buffer and transforms it
 * in-place. Stages are composed into a Pipeline and executed in order.
 */
class AudioStage {
  public:
    virtual ~AudioStage() = default;

    /**
     * @brief Process the sample buffer in-place.
     * @param samples  Interleaved float samples in [-1, 1]. Modified in-place.
     * @param numChannels  Number of interleaved channels (e.g. 2 for stereo).
     */
    virtual void process(std::vector<float> &samples, uint16_t numChannels) = 0;
};
