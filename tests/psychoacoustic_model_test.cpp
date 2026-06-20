#include <gtest/gtest.h>
#include <cmath>
#include <vector>

#include "audio/PsychoacousticModel.hpp"

namespace {

// ---------------------------------------------------------------------------
// Hz-to-Bark conversion
// ---------------------------------------------------------------------------

TEST(PsychoacousticModelTest, HzToBark_Boundaries) {
    // DC → 0 Bark
    EXPECT_NEAR(PsychoacousticModel::hzToBark(0.0f), 0.0f, 0.01f);
    // ~20 Hz → ~0.2 Bark
    EXPECT_NEAR(PsychoacousticModel::hzToBark(20.0f), 0.2f, 0.1f);
    // 1 kHz → ~8.5 Bark (standard value is 8.5)
    EXPECT_NEAR(PsychoacousticModel::hzToBark(1000.0f), 8.5f, 0.5f);
    // 4 kHz → ~17.5 Bark
    EXPECT_NEAR(PsychoacousticModel::hzToBark(4000.0f), 17.5f, 0.5f);
    // Very high frequency (20 kHz) → ~24 Bark
    float bark20k = PsychoacousticModel::hzToBark(20000.0f);
    EXPECT_GT(bark20k, 22.0f);
    EXPECT_LT(bark20k, 25.0f);
}

// ---------------------------------------------------------------------------
// Bark band centers
// ---------------------------------------------------------------------------

TEST(PsychoacousticModelTest, BarkBandCenters_Count) {
    auto centers = PsychoacousticModel::barkBandCenters();
    EXPECT_EQ(centers.size(), PsychoacousticModel::kNumBarkBands);
}

TEST(PsychoacousticModelTest, BarkBandCenters_Ascending) {
    auto centers = PsychoacousticModel::barkBandCenters();
    for (size_t i = 1; i < centers.size(); ++i) {
        EXPECT_GT(centers[i], centers[i - 1]) << " band " << i;
    }
}

// ---------------------------------------------------------------------------
// Bin to Bark band mapping
// ---------------------------------------------------------------------------

TEST(PsychoacousticModelTest, BinToBarkBand_DC) {
    size_t band = PsychoacousticModel::binToBarkBand(0, 44100, 2048);
    EXPECT_EQ(band, 0u);
}

TEST(PsychoacousticModelTest, BinToBarkBand_WithinRange) {
    // Bin at ~1 kHz with 44100 Hz sample rate and 2048 frame size:
    // freq = 1 * 44100 / 2048 ≈ 21.5 Hz → band 0
    EXPECT_EQ(PsychoacousticModel::binToBarkBand(1, 44100, 2048), 0u);

    // Bin at ~1000 Hz: 1000 * 2048 / 44100 ≈ bin 46
    // Should map to Bark band ~8
    size_t band = PsychoacousticModel::binToBarkBand(46, 44100, 2048);
    EXPECT_GE(band, 6u);
    EXPECT_LE(band, 10u);
}

TEST(PsychoacousticModelTest, BinToBarkBand_HighFreq) {
    // Nyquist bin (1024) at 44100 → 22050 Hz → should map to highest band
    size_t band = PsychoacousticModel::binToBarkBand(1024, 44100, 2048);
    EXPECT_EQ(band, PsychoacousticModel::kNumBarkBands - 1);
}

} // namespace
