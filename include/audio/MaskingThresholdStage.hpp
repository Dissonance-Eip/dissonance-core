#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "audio/AudioStage.hpp"

/**
 * @brief Shared container that holds the per-channel, per-frame psychoacoustic
 *        masking thresholds for a single audio file.
 *
 * The MaskingThresholdStage computes these masks once, upstream of the
 * perturbation stages. Downstream perturbation stages read them back (by
 * pointer to this struct) to shape their injected noise against the threshold
 * of hearing, instead of re-computing thresholds per stage.
 *
 * Threshold vectors are stored as [channel][frame], where each entry is a
 * linear-magnitude threshold vector of length frameSize (one per FFT bin).
 */
struct MaskContext {
    /** @brief Per-channel masks: [channel][frame] -> threshold vector. */
    std::vector<std::vector<std::vector<float>>> perChannelFrameThresholds;

    /** @brief FFT frame size in samples (also the per-frame vector length). */
    size_t frameSize = 2048;
    /** @brief Hop size in samples (frameSize / 2). */
    size_t hopSize = 1024;
    /** @brief Number of interleaved channels the masks were computed for. */
    uint16_t numChannels = 0;
    /** @brief Number of complete frames per channel. */
    size_t framesPerChannel = 0;

    /**
     * @brief Bounds-checked accessor for a single frame's thresholds.
     * @param channel  Channel index in [0, numChannels).
     * @param frame    Frame index in [0, framesPerChannel).
     * @return Const reference to that frame's threshold vector, or an empty
     *         vector if the indices are out of range.
     */
    const std::vector<float> &maskForChannelFrame(size_t channel, size_t frame) const {
        if (channel >= perChannelFrameThresholds.size() ||
            frame >= perChannelFrameThresholds[channel].size())
            return kEmptyThresholds;
        return perChannelFrameThresholds[channel][frame];
    }

    /** @brief True if at least one per-channel mask was produced. */
    bool hasMasks() const { return framesPerChannel > 0 && !perChannelFrameThresholds.empty(); }

  private:
    static inline const std::vector<float> kEmptyThresholds;
};

/**
 * @brief AudioStage that precomputes psychoacoustic masking thresholds once,
 *        upstream of perturbation generation.
 *
 * Given a reference (clean) audio buffer, the stage runs a windowed STFT over
 * each channel, computes the per-frame masking thresholds via
 * PsychoacousticModel::computeThresholds, scales them by maskingStrength, and
 * stores the result in a shared MaskContext for downstream stages to consume.
 *
 * Frame parameters: 2048-point FFT, 50% overlap (hop = 1024), Hann window.
 *
 * This stage is behavior-neutral: it does not modify the sample buffer. It
 * only fills the MaskContext.
 */
class MaskingThresholdStage : public AudioStage {
  public:
    /**
     * @param cleanSamples     Reference unperturbed audio (interleaved float samples).
     *                         Must outlive this stage. The data is not copied.
     * @param sampleRate       Sample rate in Hz.
     * @param numChannels      Number of interleaved channels.
     * @param maskingStrength  Threshold scale factor. 1 = normal, 0 = clamp all.
     * @param context          Shared MaskContext to write per-frame masks into. Must
     *                         outlive this stage. The data is not copied.
     * @param frameSize        FFT frame size (default 2048).
     */
    MaskingThresholdStage(const std::vector<float> &cleanSamples, uint32_t sampleRate,
                          uint16_t numChannels, float maskingStrength, MaskContext &context,
                          size_t frameSize = 2048);

    void process(std::vector<float> & /*samples*/, uint16_t /*numChannels*/) override;

    /** @brief FFT frame size. */
    size_t frameSize() const { return frameSize_; }

    /** @brief Number of frames processed in the last call to process(), per channel. */
    size_t framesPerChannel() const { return framesPerChannel_; }

  private:
    const std::vector<float> &cleanSamples_;
    uint32_t sampleRate_;
    uint16_t numChannels_;
    float maskingStrength_;
    MaskContext &context_;
    size_t frameSize_;
    size_t hopSize_;
    size_t framesPerChannel_ = 0;
};
