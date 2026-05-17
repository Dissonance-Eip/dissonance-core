#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include "audio/AudioStage.hpp"

/**
 * @brief AudioStage that applies a windowed overlap-add FFT filter.
 *
 * Splits the signal into overlapping frames (hop = frameSize / 2), applies a
 * Hann window, forward-FFTs, zeroes all bins above cutoffBin, inverse-FFTs,
 * and overlap-adds the frames back. This acts as a low-pass filter in the
 * spectral domain and forms the core of the audio protection pipeline.
 */
class WindowedFFTStage : public AudioStage {
  public:
    /**
     * @param frameSize        FFT frame length in samples (e.g. 2048).
     * @param cutoffFraction   Fraction of bins to keep [0, 1] (e.g. 0.25 keeps the
     *                         bottom quarter of the spectrum).
     * @param progressCallback Optional callback invoked with progress in [0, 1].
     */
    WindowedFFTStage(size_t frameSize, float cutoffFraction,
                     std::function<void(float)> progressCallback = {});

    void process(std::vector<float> &samples, uint16_t numChannels) override;

    /** @brief Number of FFT hops processed on the last process() call. */
    size_t framesProcessed() const { return framesProcessed_; }
    /** @brief FFT frame size (equal to the number of frequency bins). */
    size_t bins() const { return frameSize_; }
    /** @brief First bin index that was zeroed (i.e. the low-pass cutoff). */
    size_t cutoffBin() const { return cutoffBin_; }

  private:
    size_t frameSize_;
    size_t hopSize_;
    size_t cutoffBin_;
    std::vector<float> window_;
    std::vector<float> block_;
    std::function<void(float)> progressCallback_;
    size_t framesProcessed_ = 0;
};
