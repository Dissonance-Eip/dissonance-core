#pragma once

#include <cstdint>
#include <vector>

#include "audio/AudioStage.hpp"

/**
 * @brief AudioStage that injects band-limited noise above ~8 kHz.
 *
 * White noise is generated with a seeded RNG, shaped by a first-order
 * high-pass IIR filter (cutoff ~8 kHz), then mixed into the signal at a level
 * of kMaxAmplitude * strength. This sits above the speech band and disrupts
 * AI voice-cloning models without being audible to humans.
 *
 * The RNG seed should be derived from file characteristics so the perturbation
 * is repeatable for the same input but unique per file.
 */
class PerturbationStage : public AudioStage {
  public:
    /** @brief Sentinel returned by rmsDbfs() when no noise was added. */
    static constexpr float kSilentDbfs = -200.0f;

    /**
     * @param strength    Noise level in [0, 1]. 0 = off, 1 = full kMaxAmplitude above 8 kHz.
     * @param sampleRate  Sample rate of the input audio in Hz (used to compute the HP cutoff).
     * @param seed        RNG seed — derive from file characteristics for determinism.
     */
    PerturbationStage(float strength, uint32_t sampleRate, uint64_t seed = 0);

    void process(std::vector<float> &samples, uint16_t numChannels) override;

    /** @brief RMS level of the injected noise in dBFS, or kSilentDbfs if none was added. */
    float rmsDbfs() const { return rmsDbfs_; }

  private:
    float strength_;
    uint32_t sampleRate_;
    uint64_t seed_;
    float rmsDbfs_ = kSilentDbfs;

    /** @brief High-pass cutoff frequency in Hz. */
    static constexpr float kCutoffHz = 8000.0f;
    /** @brief Linear amplitude at strength = 1 (~-40 dBFS). */
    static constexpr float kMaxAmplitude = 0.01f;
};
