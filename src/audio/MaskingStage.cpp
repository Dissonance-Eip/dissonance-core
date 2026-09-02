/**
 * @file MaskingStage.cpp
 * @brief Overlap-add STFT masking stage — magnitude-domain noise injection.
 *
 * For each overlapping frame:
 *   1. Extract and window clean + perturbed audio
 *   2. FFT both to get complex spectra
 *   3. Compute perturbed magnitude and phase
 *   4. Compute clean magnitude
 *   5. Get psychoacoustic thresholds from clean magnitude
 *   6. Clamp per-bin magnitude difference to threshold
 *   7. Reconstruct complex spectrum with clamped magnitude + perturbed phase
 *   8. Inverse FFT -> overlap-add
 *
 * The result is the perturbed audio with spectral differences projected
 * back under the human hearing masking threshold.
 */

#include "audio/MaskingStage.hpp"

#include <algorithm>
#include <cmath>

#include "audio/FFTProcessor.hpp"
#include "audio/PsychoacousticModel.hpp"
#include "audio/WindowFunctions.hpp"

MaskingStage::MaskingStage(const std::vector<float> &cleanSamples, uint32_t sampleRate,
                           uint16_t numChannels, float maskingStrength, size_t frameSize,
                           MaskContext *context)
    : cleanSamples_(cleanSamples), sampleRate_(sampleRate), numChannels_(numChannels),
      frameSize_(frameSize), hopSize_(frameSize / 2), maskingStrength_(maskingStrength),
      context_(context) {}

void MaskingStage::process(std::vector<float> &samples, uint16_t numChannels) {
    if (numChannels == 0 || samples.empty() || numChannels != numChannels_)
        return;

    // Buffer sizes must match
    if (cleanSamples_.size() != samples.size())
        return;

    const size_t totalSamples = samples.size();
    const size_t totalFrames = totalSamples / numChannels;
    if (totalFrames < frameSize_) {
        framesProcessed_ = 0;
        return;
    }

    const std::vector<float> window = window::generate(window::Type::Hann, frameSize_);

    framesProcessed_ = 0;

    // Decide at most once whether to read per-frame thresholds from the shared
    // MaskContext. The shared masks are only used if they were computed for this
    // same number of channels and carry threshold vectors of this frame size.
    // NOTE: This is the NEW behaviour (issue #87 wiring): when a compatible shared
    // mask is available, MaskingStage consumes it instead of recomputing thresholds.
    // The legacy per-frame recompute path is kept below but is NOT the active path
    // when a valid shared mask is present.
    const bool useSharedMask =
        context_ != nullptr && context_->numChannels == numChannels_ && context_->hasMasks();

    // Process each channel independently
    for (uint16_t ch = 0; ch < numChannels_; ++ch) {
        std::vector<float> output(totalFrames, 0.0f);

        for (size_t offset = 0; offset + frameSize_ <= totalFrames;
             offset += hopSize_, ++framesProcessed_) {

            // ── Extract clean frame ──
            std::vector<float> cleanFrame(frameSize_);
            for (size_t i = 0; i < frameSize_; ++i)
                cleanFrame[i] = cleanSamples_[(offset + i) * numChannels + ch];

            // ── Extract perturbed frame ──
            std::vector<float> pertFrame(frameSize_);
            for (size_t i = 0; i < frameSize_; ++i)
                pertFrame[i] = samples[(offset + i) * numChannels + ch];

            // ── Window both ──
            window::apply(cleanFrame, window);
            window::apply(pertFrame, window);

            // ── Forward FFT both ──
            auto cleanSpectrum = fft::transform(cleanFrame);
            auto pertSpectrum = fft::transform(pertFrame);

            // ── Compute clean magnitude and perturbed magnitude/phase ──
            std::vector<float> cleanMag(frameSize_);
            std::vector<float> pertMag(frameSize_);
            std::vector<float> pertPhase(frameSize_);

            for (size_t i = 0; i < frameSize_; ++i) {
                cleanMag[i] = std::abs(cleanSpectrum[i]);
                pertMag[i] = std::abs(pertSpectrum[i]);
                pertPhase[i] = std::arg(pertSpectrum[i]);
            }

            // ── Obtain per-frame masking thresholds ──
            // NEW path: read from the shared MaskContext (already scaled by
            // maskingStrength). The frame index within the channel is offset/hop.
            // Fall back to legacy recompute if the shared mask is unavailable or
            // does not match this frame geometry.
            const size_t maskFrameIndex = offset / hopSize_;
            std::vector<float> thresholds;
            if (useSharedMask) {
                const std::vector<float> &shared =
                    context_->maskForChannelFrame(ch, maskFrameIndex);
                if (shared.size() == frameSize_) {
                    thresholds = shared;
                }
            }
            if (thresholds.empty()) {
                // ── LEGACY (not the active path when a shared mask is present) ──
                // Recompute the thresholds from the clean magnitude each frame.
                // NOTE: this code is intentionally retained for reference/fallback
                // and is NOT what runs when MaskingStage is fed from the shared
                // MaskContext (see useSharedMask above).
                thresholds =
                    PsychoacousticModel::computeThresholds(cleanMag, sampleRate_, frameSize_);

                // ── Scale thresholds by masking strength ──
                if (maskingStrength_ < 1.0f) {
                    std::transform(thresholds.begin(), thresholds.end(), thresholds.begin(),
                                   [this](float t) { return t * maskingStrength_; });
                }
            }

            // ── Clamp per-bin perturbation to threshold ──
            std::vector<std::complex<float>> clampedSpectrum = pertSpectrum;
            for (size_t i = 0; i < frameSize_; ++i) {
                const float diff = pertMag[i] - cleanMag[i];
                const float thresh = thresholds[i];

                if (diff > thresh) {
                    // Positive perturbation exceeds threshold: clamp to clean + thresh
                    const float clampedMag = cleanMag[i] + thresh;
                    clampedSpectrum[i] = std::complex<float>(clampedMag * std::cos(pertPhase[i]),
                                                             clampedMag * std::sin(pertPhase[i]));
                } else if (diff < -thresh) {
                    // Negative perturbation exceeds threshold: clamp to clean - thresh
                    const float clampedMag = std::max(cleanMag[i] - thresh, 0.0f);
                    clampedSpectrum[i] = std::complex<float>(clampedMag * std::cos(pertPhase[i]),
                                                             clampedMag * std::sin(pertPhase[i]));
                }
                // else: |diff| <= thresh -> keep original pertSpectrum (no modification)
            }

            // ── Inverse FFT ──
            auto reconstructed = fft::inverse(clampedSpectrum);

            // ── Overlap-add ──
            // Hann window at 50% overlap satisfies COLA: summing frames
            // reconstructs the original signal without normalization.
            for (size_t i = 0; i < frameSize_; ++i) {
                const size_t idx = offset + i;
                if (idx < totalFrames)
                    output[idx] += reconstructed[i];
            }
        }

        // ── Write back ──
        for (size_t i = 0; i < totalFrames; ++i) {
            samples[i * numChannels + ch] = std::clamp(output[i], -1.0f, 1.0f);
        }
    }
}
