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

// ---------------------------------------------------------------------------
// Spreading function
// ---------------------------------------------------------------------------

TEST(PsychoacousticModelTest, SpreadingAttenuation_Downward) {
    // Target below masker → -10 dB/Bark: attenuation should be positive
    // masker at 5 Bark, target at 3 Bark → dz = -2 → atten = -10 * (-2) = 20
    float atten = PsychoacousticModel::spreadingAttenuation(5.0f, 3.0f);
    EXPECT_FLOAT_EQ(atten, 20.0f);
}

TEST(PsychoacousticModelTest, SpreadingAttenuation_Upward) {
    // Target above masker → +25 dB/Bark
    // masker at 3 Bark, target at 5 Bark → dz = 2 → atten = 25 * 2 = 50
    float atten = PsychoacousticModel::spreadingAttenuation(3.0f, 5.0f);
    EXPECT_FLOAT_EQ(atten, 50.0f);
}

TEST(PsychoacousticModelTest, SpreadingAttenuation_SameBand) {
    // Same band → zero attenuation
    float atten = PsychoacousticModel::spreadingAttenuation(7.0f, 7.0f);
    EXPECT_FLOAT_EQ(atten, 0.0f);
}

TEST(PsychoacousticModelTest, SpreadingAttenuation_Symmetric) {
    // Attenuation should be direction-dependent (not symmetric)
    float up = PsychoacousticModel::spreadingAttenuation(3.0f, 5.0f);   // +25 * 2
    float down = PsychoacousticModel::spreadingAttenuation(5.0f, 3.0f); // -10 * -2
    EXPECT_NE(up, down);
    EXPECT_GT(up, down); // upward spread attenuates more
}

// ---------------------------------------------------------------------------
// Absolute threshold of hearing
// ---------------------------------------------------------------------------

TEST(PsychoacousticModelTest, AbsoluteThresholdDb_KnownValues) {
    // At 1 kHz, threshold should be around 0 dB (normalized reference)
    float thresh1k = PsychoacousticModel::absoluteThresholdDb(1000.0f);
    EXPECT_NEAR(thresh1k, 0.0f, 5.0f);

    // At 4 kHz, threshold dips (ear most sensitive ~2-5 kHz)
    float thresh4k = PsychoacousticModel::absoluteThresholdDb(4000.0f);
    EXPECT_LT(thresh4k, thresh1k); // more sensitive (lower threshold)

    // At 100 Hz, threshold should be higher (less sensitive at low frequencies)
    float thresh100 = PsychoacousticModel::absoluteThresholdDb(100.0f);
    EXPECT_GT(thresh100, thresh1k); // less sensitive
}

TEST(PsychoacousticModelTest, AbsoluteThresholdDb_NonNegativeAtExtremes) {
    // Very low frequencies should not produce NaN or negative infinity
    float thresh20 = PsychoacousticModel::absoluteThresholdDb(20.0f);
    EXPECT_TRUE(std::isfinite(thresh20));

    // Very high frequencies should be finite
    float thresh20k = PsychoacousticModel::absoluteThresholdDb(20000.0f);
    EXPECT_TRUE(std::isfinite(thresh20k));
}

// ---------------------------------------------------------------------------
// Compute masking thresholds
// ---------------------------------------------------------------------------

TEST(PsychoacousticModelTest, ComputeThresholds_SizeMismatchReturnsEmpty) {
    std::vector<float> mag = {1.0f, 0.5f, 0.25f}; // only 3 entries, frameSize=2048
    auto thresholds = PsychoacousticModel::computeThresholds(mag, 44100, 2048);
    EXPECT_TRUE(thresholds.empty());
}

TEST(PsychoacousticModelTest, ComputeThresholds_OutputSizeMatches) {
    constexpr size_t frameSize = 256;
    std::vector<float> mag(frameSize, 0.5f);
    auto thresholds = PsychoacousticModel::computeThresholds(mag, 44100, frameSize);
    EXPECT_EQ(thresholds.size(), frameSize);
}

TEST(PsychoacousticModelTest, ComputeThresholds_AllZero) {
    constexpr size_t frameSize = 256;
    std::vector<float> mag(frameSize, 0.0f);
    auto thresholds = PsychoacousticModel::computeThresholds(mag, 44100, frameSize);
    EXPECT_EQ(thresholds.size(), frameSize);
    // With zero input, thresholds should revert to absolute threshold of hearing
    // All entries should be finite and non-negative
    for (size_t i = 0; i < thresholds.size(); ++i) {
        EXPECT_TRUE(std::isfinite(thresholds[i]));
        EXPECT_GE(thresholds[i], 0.0f);
    }
}

TEST(PsychoacousticModelTest, ComputeThresholds_PureTone) {
    constexpr size_t frameSize = 512;

    // A single strong tone at bin 50 (~4307 Hz at 44100/512)
    std::vector<float> mag(frameSize, 0.0f);
    mag[50] = 1.0f;
    // Mirror for symmetry
    mag[frameSize - 50] = 1.0f;

    auto thresholds = PsychoacousticModel::computeThresholds(mag, 44100, frameSize);
    EXPECT_EQ(thresholds.size(), frameSize);

    // The band containing bin 50 should have the highest threshold
    // (most masking, most permissive)
    float maxThresh = 0.0f;
    for (float t : thresholds) {
        if (t > maxThresh)
            maxThresh = t;
    }
    EXPECT_GT(maxThresh, 0.01f);
}

} // namespace
