#include "audio/WindowedFFTStage.hpp"

#include <algorithm>

#include "audio/FFTProcessor.hpp"
#include "audio/WindowFunctions.hpp"

WindowedFFTStage::WindowedFFTStage(size_t frameSize, float cutoffFraction,
                                   std::function<void(float)> progressCallback)
    : frameSize_(frameSize), hopSize_(frameSize / 2),
      cutoffBin_(static_cast<size_t>(cutoffFraction * static_cast<float>(frameSize))),
      window_(window::generate(window::Type::Hann, frameSize)), block_(frameSize, 0.0f),
      progressCallback_(std::move(progressCallback)) {}

void WindowedFFTStage::process(std::vector<float> &samples, uint16_t numChannels) {
    if (numChannels == 0 || samples.empty())
        return;

    const size_t totalFrames = samples.size() / numChannels;
    if (totalFrames < 2)
        return;

    for (uint16_t ch = 0; ch < numChannels; ++ch) {
        std::vector<float> output(totalFrames, 0.0f);
        size_t hopIdx = 0;

        for (size_t offset = 0; offset < totalFrames; offset += hopSize_, ++hopIdx) {
            if (ch == 0 && progressCallback_ && hopIdx % 16 == 0)
                progressCallback_(static_cast<float>(offset) / static_cast<float>(totalFrames));

            std::fill(block_.begin(), block_.end(), 0.0f);
            const size_t available = std::min(frameSize_, totalFrames - offset);
            for (size_t i = 0; i < available; ++i)
                block_[i] = samples[(offset + i) * numChannels + ch];

            window::apply(block_, window_);
            auto spectrum = fft::transform(block_);

            for (size_t k = cutoffBin_; k < spectrum.size(); ++k)
                spectrum[k] = {0.0, 0.0};

            auto reconstructed = fft::inverse(spectrum);
            for (size_t i = 0; i < frameSize_ && (offset + i) < totalFrames; ++i)
                output[offset + i] += static_cast<float>(reconstructed[i]);
        }

        for (size_t i = 0; i < totalFrames; ++i)
            samples[i * numChannels + ch] = std::clamp(output[i], -1.0f, 1.0f);
    }

    if (progressCallback_)
        progressCallback_(1.0f);

    framesProcessed_ = totalFrames;
}
