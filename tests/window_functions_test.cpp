#include <gtest/gtest.h>
#include <cmath>
#include <vector>

#include "audio/WindowFunctions.hpp"

constexpr double EPSILON = 1e-6;

class WindowFunctionsTest : public ::testing::Test {
  protected:
    // Helper to check if two doubles are approximately equal
    static bool approxEqual(double a, double b, double epsilon = EPSILON) {
        return std::abs(a - b) < epsilon;
    }
};

// Test Hann window generation
TEST_F(WindowFunctionsTest, HannWindowSize8) {
    std::vector<double> window = WindowFunctions::generate(WindowFunctions::Type::Hann, 8);

    ASSERT_EQ(window.size(), 8);

    // Hann window should taper to 0 at both ends
    EXPECT_TRUE(approxEqual(window[0], 0.0));
    EXPECT_TRUE(approxEqual(window[7], 0.0));

    // Should be symmetric
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_TRUE(approxEqual(window[i], window[7 - i]));
    }

    // Center values should be higher (approaching 1.0)
    EXPECT_GT(window[3], 0.9);
    EXPECT_GT(window[4], 0.9);
}

TEST_F(WindowFunctionsTest, HannWindowSize512) {
    std::vector<double> window = WindowFunctions::generate(WindowFunctions::Type::Hann, 512);

    ASSERT_EQ(window.size(), 512);

    // Edges should be 0
    EXPECT_TRUE(approxEqual(window[0], 0.0));
    EXPECT_TRUE(approxEqual(window[511], 0.0));

    // Center should be very close to 1.0
    EXPECT_TRUE(approxEqual(window[256], 1.0, 1e-4));

    // Check symmetry
    for (size_t i = 0; i < 256; ++i) {
        EXPECT_TRUE(approxEqual(window[i], window[511 - i]));
    }
}

TEST_F(WindowFunctionsTest, HannWindowSize1) {
    std::vector<double> window = WindowFunctions::generate(WindowFunctions::Type::Hann, 1);

    ASSERT_EQ(window.size(), 1);
    EXPECT_EQ(window[0], 1.0);
}

// Test Hamming window generation
TEST_F(WindowFunctionsTest, HammingWindowSize8) {
    std::vector<double> window = WindowFunctions::generate(WindowFunctions::Type::Hamming, 8);

    ASSERT_EQ(window.size(), 8);

    // Hamming window should NOT go to 0 at edges (approximately 0.08)
    EXPECT_TRUE(approxEqual(window[0], 0.08, 0.01));
    EXPECT_TRUE(approxEqual(window[7], 0.08, 0.01));

    // Should be symmetric
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_TRUE(approxEqual(window[i], window[7 - i]));
    }

    // Center values should be higher
    EXPECT_GT(window[3], 0.9);
    EXPECT_GT(window[4], 0.9);
}

TEST_F(WindowFunctionsTest, HammingWindowSize512) {
    std::vector<double> window = WindowFunctions::generate(WindowFunctions::Type::Hamming, 512);

    ASSERT_EQ(window.size(), 512);

    // Edges should be ~0.08
    EXPECT_TRUE(approxEqual(window[0], 0.08, 0.01));
    EXPECT_TRUE(approxEqual(window[511], 0.08, 0.01));

    // Center should be very close to 1.0
    EXPECT_TRUE(approxEqual(window[256], 1.0, 1e-4));

    // Check symmetry
    for (size_t i = 0; i < 256; ++i) {
        EXPECT_TRUE(approxEqual(window[i], window[511 - i]));
    }
}

TEST_F(WindowFunctionsTest, HammingWindowSize1) {
    std::vector<double> window = WindowFunctions::generate(WindowFunctions::Type::Hamming, 1);

    ASSERT_EQ(window.size(), 1);
    EXPECT_EQ(window[0], 1.0);
}

// Test window values are in valid range [0, 1]
TEST_F(WindowFunctionsTest, WindowValuesInRange) {
    std::vector<double> hannWindow = WindowFunctions::generate(WindowFunctions::Type::Hann, 256);
    std::vector<double> hammingWindow =
        WindowFunctions::generate(WindowFunctions::Type::Hamming, 256);

    for (double val : hannWindow) {
        EXPECT_GE(val, 0.0);
        EXPECT_LE(val, 1.0);
    }

    for (double val : hammingWindow) {
        EXPECT_GE(val, 0.0);
        EXPECT_LE(val, 1.0);
    }
}

// Test apply function with double samples
TEST_F(WindowFunctionsTest, ApplyWindowToDoubles) {
    std::vector<double> samples = {1000.0, 2000.0, 3000.0, 4000.0, 3000.0, 2000.0, 1000.0, 500.0};
    std::vector<double> window = WindowFunctions::generate(WindowFunctions::Type::Hann, 8);

    std::vector<double> original = samples;
    WindowFunctions::apply(samples, window);

    // Edges should be attenuated to near zero
    EXPECT_TRUE(approxEqual(samples[0], 0.0, 1.0));
    EXPECT_TRUE(approxEqual(samples[7], 0.0, 1.0));

    // Middle values should be less than original but not zero
    EXPECT_LT(samples[3], original[3]);
    EXPECT_GT(samples[3], 0.0);
}

// Test apply function with int16_t samples
TEST_F(WindowFunctionsTest, ApplyWindowToInt16) {
    std::vector<int16_t> samples = {1000, 2000, 3000, 4000, 3000, 2000, 1000, 500};
    std::vector<double> window = WindowFunctions::generate(WindowFunctions::Type::Hann, 8);

    std::vector<int16_t> original = samples;
    WindowFunctions::apply(samples, window);

    // Edges should be attenuated to near zero
    EXPECT_NEAR(samples[0], 0, 1);
    EXPECT_NEAR(samples[7], 0, 1);

    // Middle values should be attenuated but not zero
    EXPECT_LT(samples[3], original[3]);
    EXPECT_GT(samples[3], 0);
}

// Test error handling
TEST_F(WindowFunctionsTest, InvalidWindowSize) {
    EXPECT_THROW(
        { WindowFunctions::generate(WindowFunctions::Type::Hann, 0); }, std::invalid_argument);
}

TEST_F(WindowFunctionsTest, MismatchedSizesDouble) {
    std::vector<double> samples = {1.0, 2.0, 3.0, 4.0};
    std::vector<double> window = {0.5, 0.5, 0.5};

    EXPECT_THROW({ WindowFunctions::apply(samples, window); }, std::invalid_argument);
}

TEST_F(WindowFunctionsTest, MismatchedSizesInt16) {
    std::vector<int16_t> samples = {1, 2, 3, 4};
    std::vector<double> window = {0.5, 0.5, 0.5};

    EXPECT_THROW({ WindowFunctions::apply(samples, window); }, std::invalid_argument);
}

// Test clamping behavior for int16_t
TEST_F(WindowFunctionsTest, Int16ClampingBehavior) {
    std::vector<int16_t> samples = {32767, -32768, 30000, -30000};
    std::vector<double> window = {1.0, 1.0, 0.5, 0.5};

    WindowFunctions::apply(samples, window);

    // Values should stay within int16_t range
    EXPECT_LE(samples[0], std::numeric_limits<int16_t>::max());
    EXPECT_GE(samples[1], std::numeric_limits<int16_t>::min());
}
