#include "audio/Pipeline.hpp"

void Pipeline::addStage(std::unique_ptr<AudioStage> stage) { stages_.push_back(std::move(stage)); }

void Pipeline::run(std::vector<float> &samples, uint16_t numChannels) {
    for (auto &stage : stages_)
        stage->process(samples, numChannels);
}
