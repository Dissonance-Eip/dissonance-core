/**
 * @file MaskingThresholdStage.cpp
 * @brief Precomputes per-channel, per-frame psychoacoustic masking thresholds
 *        once, upstream of perturbation generation.
 *
 * For each channel and each overlapping frame:
 *   1. Extract and window the clean frame
 *   2. Forward-FFT to get the complex spectrum
 *   3. Compute the clean magnitude
 *   4. Get the psychoacoustic thresholds from that magnitude
 *   5. Scale thresholds by maskingStrength
 *   6. Store the per-frame threshold vector in the shared MaskContext
 *
 * The sample buffer is left unmodified, so this stage is behavior-neutral. The
 * stored masks are consumed by downstream perturbation stages.
 */

#include "audio/MaskingThresholdStage.hpp"

#include <algorithm>

#include "audio/FFTProcessor.hpp"
#include "audio/PsychoacousticModel.hpp"
#include "audio/WindowFunctions.hpp"

MaskingThresholdStage::MaskingThresholdStage(const std::vector<float> &cleanSamples,
                                             uint32_t sampleRate, uint16_t numChannels,
                                             float maskingStrength, MaskContext &context,
                                             size_t frameSize)
    : cleanSamples_(cleanSamples), sampleRate_(sampleRate), numChannels_(numChannels),
      maskingStrength_(maskingStrength), context_(context), frameSize_(frameSize),
      hopSize_(frameSize / 2) {}

void MaskingThresholdStage::process(std::vector<float> & /*samples*/, uint16_t /*numChannels*/) {
    if (numChannels_ == 0 || cleanSamples_.empty())
        return;

    const size_t totalSamples = cleanSamples_.size();
    const size_t totalFrames = totalSamples / numChannels_;
    if (totalFrames < frameSize_) {
        framesPerChannel_ = 0;
        return;
    }

    const std::vector<float> window = window::generate(window::Type::Hann, frameSize_);

    const size_t numFrames = (totalFrames - frameSize_) / hopSize_ + 1;
    framesPerChannel_ = numFrames;

    // Reset the shared context and fill it afresh.
    context_.frameSize = frameSize_;
    context_.hopSize = hopSize_;
    context_.numChannels = numChannels_;
    context_.framesPerChannel = numFrames;
    context_.perChannelFrameThresholds.assign(numChannels_,
                                              std::vector<std::vector<float>>(numFrames));

    // Process each channel independently, mirroring MaskingStage.
    for (uint16_t ch = 0; ch < numChannels_; ++ch) {
        for (size_t f = 0; f < numFrames; ++f) {
            const size_t offset = f * hopSize_;

            std::vector<float> cleanFrame(frameSize_);
            for (size_t i = 0; i < frameSize_; ++i)
                cleanFrame[i] = cleanSamples_[(offset + i) * numChannels_ + ch];

            window::apply(cleanFrame, window);

            auto spectrum = fft::transform(cleanFrame);

            std::vector<float> cleanMag(frameSize_);
            for (size_t i = 0; i < frameSize_; ++i)
                cleanMag[i] = std::abs(spectrum[i]);

            std::vector<float> thresholds =
                PsychoacousticModel::computeThresholds(cleanMag, sampleRate_, frameSize_);

            if (maskingStrength_ < 1.0f) {
                std::transform(thresholds.begin(), thresholds.end(), thresholds.begin(),
                               [this](float t) { return t * maskingStrength_; });
            }

            context_.perChannelFrameThresholds[ch][f] = std::move(thresholds);
        }
    }
}
