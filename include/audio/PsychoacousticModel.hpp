#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * @brief Psychoacoustic model providing Bark-scale critical band mapping.
 *
 * Maps FFT bin indices to 24 Bark critical bands (0..23) using the standard
 * Zwicker & Terhardt formula. Stateless — all methods are static.
 */
class PsychoacousticModel {
  public:
    /** @brief Number of Bark critical bands (0..23). */
    static constexpr size_t kNumBarkBands = 24;

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
};
