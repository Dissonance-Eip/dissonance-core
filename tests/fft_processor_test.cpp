#include <gtest/gtest.h>
#include <vector>
#include <complex>

#include "audio/FFTProcessor.hpp"
#include "core/Errors.hpp"

constexpr double EPS = 1e-6;

TEST(FFTProcessorTest, ImpulseHasFlatSpectrum) {
    std::vector<double> impulse = {1.0, 0.0, 0.0, 0.0};
    auto spectrum = fft::transform(impulse);

    ASSERT_EQ(spectrum.size(), impulse.size());
    for (const auto &bin : spectrum) {
        EXPECT_NEAR(bin.real(), 1.0, EPS);
        EXPECT_NEAR(bin.imag(), 0.0, EPS);
    }
}

TEST(FFTProcessorTest, InverseReconstructsImpulse) {
    std::vector<double> impulse = {1.0, 0.0, 0.0, 0.0};
    auto spectrum = fft::transform(impulse);
    auto time = fft::inverse(spectrum);

    ASSERT_EQ(time.size(), impulse.size());
    for (size_t i = 0; i < time.size(); ++i) {
        EXPECT_NEAR(time[i], impulse[i], EPS);
    }
}

TEST(FFTProcessorTest, RoundTripSignal) {
    std::vector<double> signal = {0.0, 1.0, 0.0, -1.0};
    auto spectrum = fft::transform(signal);
    auto time = fft::inverse(spectrum);

    ASSERT_EQ(time.size(), signal.size());
    for (size_t i = 0; i < time.size(); ++i) {
        EXPECT_NEAR(time[i], signal[i], EPS);
    }
}

TEST(FFTProcessorTest, MagnitudeMatchesSize) {
    std::vector<double> signal = {0.0, 1.0, 0.0, -1.0};
    auto spectrum = fft::transform(signal);
    auto mags = fft::magnitude(spectrum);

    EXPECT_EQ(mags.size(), spectrum.size());
}

TEST(FFTProcessorTest, ThrowsOnEmptyInput) {
    std::vector<double> emptyReal;
    std::vector<std::complex<double>> emptyComplex;

    EXPECT_THROW(fft::transform(emptyReal), dissonance::DspError);
    EXPECT_THROW(fft::inverse(emptyComplex), dissonance::DspError);
}
