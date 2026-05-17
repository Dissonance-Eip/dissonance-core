#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "audio/AudioStage.hpp"

/**
 * @brief Ordered chain of AudioStage processors.
 *
 * Stages are run sequentially on the same sample buffer. Each stage
 * receives the output of the previous one, allowing effects to compose.
 */
class Pipeline {
  public:
    /** @brief Append a stage to the end of the chain. */
    void addStage(std::unique_ptr<AudioStage> stage);

    /** @brief Execute all stages in order on the sample buffer. */
    void run(std::vector<float> &samples, uint16_t numChannels);

  private:
    std::vector<std::unique_ptr<AudioStage>> stages_;
};
