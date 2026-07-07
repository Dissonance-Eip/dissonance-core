#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "audio/AudioStage.hpp"

/**
 * @brief AudioStage that clamps spectral perturbation under psychoacoustic
 *        masking thresholds (magnitude-domain noise injection).
 *
 * Given a reference (clean) audio buffer, MaskingStage computes the STFT of
 * both the clean and the current (perturbed) audio, then for each
 * time-frequency bin clamps the spectral magnitude difference so that it
 * does not exceed the masking threshold derived from the clean signal.
 *
 * Frame parameters: 2048-point FFT, 50% overlap (hop = 1024), Hann window.
 */
class MaskingStage : public AudioStage {
  public:
    /**
     * @param cleanSamples     Reference unperturbed audio (interleaved float samples).
     *                         Must outlive this stage. The data is not copied.
     * @param sampleRate       Sample rate in Hz.
     * @param numChannels      Number of interleaved channels.
     * @param maskingStrength  Threshold scale factor. 1 = normal, 0 = clamp all.
     * @param frameSize        FFT frame size (default 2048).
     */
    MaskingStage(const std::vector<float> &cleanSamples, uint32_t sampleRate, uint16_t numChannels,
                 float maskingStrength, size_t frameSize = 2048);

    void process(std::vector<float> &samples, uint16_t numChannels) override;

    /** @brief FFT frame size. */
    size_t frameSize() const { return frameSize_; }

    /** @brief Number of frames processed in the last call to process(). */
    size_t framesProcessed() const { return framesProcessed_; }

  private:
    const std::vector<float> &cleanSamples_;
    uint32_t sampleRate_;
    uint16_t numChannels_;
    size_t frameSize_;
    size_t hopSize_;
    float maskingStrength_;
    size_t framesProcessed_ = 0;
};
