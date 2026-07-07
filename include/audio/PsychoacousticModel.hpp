#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * @brief Psychoacoustic model for computing per-bin masking thresholds.
 *
 * Maps FFT bins to 24 Bark critical bands and applies a triangular spreading
 * function.
 *
 * The main entry point is computeThresholds(), which takes a magnitude spectrum
 * and returns per-bin linear masking thresholds suitable for clamping
 * perturbation magnitudes.
 */
class PsychoacousticModel {
  public:
    /** @brief Number of Bark critical bands (0..23). */
    static constexpr size_t kNumBarkBands = 24;

    // --- Threshold computation (main entry point) ---

    /**
     * @brief Compute per-bin masking thresholds for a single FFT frame.
     *
     * Accumulates energy per Bark band, applies triangular spreading, floors
     * the result, and maps back to per-bin linear magnitude thresholds.
     *
     * @param magnitude  Magnitude spectrum (full frameSize bins, linear scale).
     *                   Must have length == frameSize.
     * @param sampleRate Sample rate in Hz.
     * @param frameSize  FFT frame size in samples (must be even).
     * @return Threshold vector (same size as magnitude, linear scale).
     *         Each entry is the maximum allowed linear magnitude perturbation.
     */
    static std::vector<float> computeThresholds(const std::vector<float> &magnitude,
                                                uint32_t sampleRate, size_t frameSize);

    // --- Utility functions (public for testability / CLI) ---

    /** @brief Convert frequency in Hz to Bark-scale value. */
    static float hzToBark(float hz);

    /** @brief Compute the 24 Bark band center frequencies in Hz. */
    static std::vector<float> barkBandCenters();

    /**
     * @brief Map an FFT bin index to its Bark band.
     * @param binIndex   FFT bin index (0 = DC).
     * @param sampleRate Sample rate in Hz.
     * @param frameSize  FFT frame size in samples.
     * @return Bark band index in [0, kNumBarkBands - 1].
     */
    static size_t binToBarkBand(size_t binIndex, uint32_t sampleRate, size_t frameSize);

    /**
     * @brief Spreading function attenuation between two Bark values.
     *
     * Triangular approximation of the "upward spread of masking": a masker
     * suppresses content above its own frequency more effectively than
     * below it, so the upward slope (~10 dB/Bark) is gentler than the
     * downward slope (~25 dB/Bark).
     *
     * @param maskerBark Bark value of the masker (source) band.
     * @param targetBark Bark value of the target band.
     * @return Attenuation in dB (always >= 0).
     */
    static float spreadingAttenuation(float maskerBark, float targetBark);

  private:
    /**
     * @brief Apply triangular spreading across Bark bands.
     *
     * Convolves each band's energy (in dB) with the spreading function,
     * taking the per-band maximum (peak-picking model).
     *
     * @param bandEnergiesDB 24-element vector of Bark band energies in dB.
     * @return Spread masking threshold per band in dB SPL.
     */
    static std::vector<float> applySpreading(const std::vector<float> &bandEnergiesDB);
};
