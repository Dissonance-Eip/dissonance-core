#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "audio/AudioStage.hpp"

/**
 * @brief AudioStage that injects adversarial noise using one of several
 *        perturbation strategies.
 *
 * Supported modes:
 *   - "white_noise"      — HP-filtered white noise above 8 kHz (original algorithm).
 *   - "phase_distortion" — Noise shaped by FFT phase randomisation per frame.
 *   - "spectral_gate"    — Noise with randomly zeroed spectral bins per frame.
 *   - "pink_noise"       — Voss-McCartney 1/f noise + HP filter.
 *
 * The RNG seed should be derived from file characteristics so the perturbation
 * is repeatable for the same input but unique per file.
 */
class PerturbationStage : public AudioStage {
  public:
    /** @brief Sentinel returned by rmsDbfs() when no noise was added. */
    static constexpr float kSilentDbfs = -200.0f;

    /**
     * @param mode        Perturbation strategy: "white_noise", "phase_distortion",
     *                    "spectral_gate", or "pink_noise".
     * @param strength    Noise level multiplier in [0, 1].
     * @param sampleRate  Sample rate of the input audio in Hz.
     * @param seed        RNG seed — derive from file characteristics for determinism.
     */
    PerturbationStage(const std::string &mode, float strength, uint32_t sampleRate,
                      uint64_t seed = 0);

    void process(std::vector<float> &samples, uint16_t numChannels) override;

    /** @brief RMS level of the injected noise in dBFS, or kSilentDbfs if none was added. */
    float rmsDbfs() const { return rmsDbfs_; }

  private:
    static void applyWhiteNoise(const std::vector<float> &noise, uint16_t numChannels);
    void applyPhaseDistortion(std::vector<float> &noise, uint16_t numChannels);
    void applySpectralGate(std::vector<float> &noise, uint16_t numChannels);
    void applyPinkNoise(std::vector<float> &noise, uint16_t numChannels);

    void generateWhiteNoise(std::vector<float> &buffer);
    void highPassFilter(std::vector<float> &buffer, uint16_t numChannels);

    std::string mode_;
    float strength_;
    uint32_t sampleRate_;
    uint64_t seed_;
    float rmsDbfs_ = kSilentDbfs;

    /** @brief Frame size for FFT-based modes. */
    static constexpr size_t kFrameSize = 2048;
    /** @brief Hop size (50 % overlap). */
    static constexpr size_t kHopSize = kFrameSize / 2;
    /** @brief High-pass cutoff frequency in Hz. */
    static constexpr float kCutoffHz = 8000.0f;
    /** @brief Linear amplitude at strength = 1 (~-40 dBFS). */
    static constexpr float kMaxAmplitude = 0.01f;
};
