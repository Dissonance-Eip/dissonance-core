/**
 * @file PsychoacousticModel.cpp
 * @brief Bark-scale critical band mapping implementation.
 *
 * This file implements the Bark-scale frequency scale used in psychoacoustics.
 * It provides Hz-to-Bark conversion and FFT-bin-to-Bark-band mapping.
 */

#include "audio/PsychoacousticModel.hpp"

#include <algorithm>
#include <cmath>

// ============================================================================
// Hz-to-Bark conversion
// ============================================================================

float PsychoacousticModel::hzToBark(float hz) {
    if (hz <= 0.0f)
        return 0.0f;
    // Standard formula from Zwicker & Terhardt
    return 13.0f * std::atan(0.00076f * hz) + 3.5f * std::atan(std::pow(hz / 7500.0f, 2.0f));
}

// ============================================================================
// Bark band center frequencies
// ============================================================================

std::vector<float> PsychoacousticModel::barkBandCenters() {
    // Center frequencies (Hz) of the 24 Bark-scale critical bands
    // Standard Zwicker values
    static const float kCenters[kNumBarkBands] = {
        50.0f,   150.0f,  250.0f,  350.0f,  450.0f,  570.0f,  700.0f,   840.0f,
        1000.0f, 1170.0f, 1370.0f, 1600.0f, 1850.0f, 2150.0f, 2500.0f,  2900.0f,
        3400.0f, 4000.0f, 4800.0f, 5800.0f, 7000.0f, 8500.0f, 10500.0f, 13500.0f};
    return std::vector<float>(kCenters, kCenters + kNumBarkBands);
}

// ============================================================================
// FFT bin to Bark band mapping
// ============================================================================

size_t PsychoacousticModel::binToBarkBand(size_t binIndex, uint32_t sampleRate, size_t frameSize) {
    if (binIndex == 0)
        return 0;

    const float hz = static_cast<float>(binIndex) * static_cast<float>(sampleRate) /
                     static_cast<float>(frameSize);
    const float bark = hzToBark(hz);

    // Clamp to [0, kNumBarkBands - 1]
    size_t band = static_cast<size_t>(std::floor(bark));
    if (band >= kNumBarkBands)
        band = kNumBarkBands - 1;
    return band;
}
