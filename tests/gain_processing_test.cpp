#include <gtest/gtest.h>
#include <cmath>
#include <fstream>
#include <limits>

#include "WavProcessor.hpp"
#include "WavParser.hpp"

class GainProcessingTest : public ::testing::Test {
protected:
    std::string testFile = "./test_files/sound.wav";

    void SetUp() override {
        // Verify test file exists
        std::ifstream file(testFile, std::ios::binary);
        if (!file.is_open()) {
            SKIP() << "Test file not found: " << testFile;
        }
    }
};

TEST_F(GainProcessingTest, DefaultGainProcessing) {
    ASSERT_NO_THROW({
        ProcessedWav result = processWavFile(testFile);
        
        EXPECT_EQ(result.originalSamples.size(), result.processedSamples.size());
        EXPECT_FALSE(result.processedPath.empty());
        EXPECT_TRUE(result.processedPath.find("-processed") != std::string::npos);
    });
}

TEST_F(GainProcessingTest, GainPointFive) {
    ProcessedWav result = processWavFile(testFile, 0.5);
    
    ASSERT_EQ(result.originalSamples.size(), result.processedSamples.size());
    
    // Verify gain was applied by checking a few non-zero samples
    int verified = 0;
    const auto& original = result.originalSamples;
    const auto& processed = result.processedSamples;

    for (size_t i = 0; i < original.size(); ++i) {
        if (original[i] != 0) {
            int32_t expected = static_cast<int32_t>(
                std::lround(static_cast<double>(original[i]) * 0.5)
            );
            expected = std::clamp(expected, 
                                 static_cast<int32_t>(std::numeric_limits<int16_t>::min()),
                                 static_cast<int32_t>(std::numeric_limits<int16_t>::max()));
            
            EXPECT_EQ(processed[i], static_cast<int16_t>(expected))
                << "Mismatch at sample " << i;
            verified++;
        }
    }
}

TEST_F(GainProcessingTest, GainOnePointZero) {
    ProcessedWav result = processWavFile(testFile, 1.0);
    
    ASSERT_EQ(result.originalSamples.size(), result.processedSamples.size());
    
    // With gain 1.0, samples should be identical
    for (size_t i = 0; i < result.originalSamples.size(); ++i) {
        EXPECT_EQ(result.originalSamples[i], result.processedSamples[i])
            << "Sample " << i << " should be unchanged with gain 1.0";
    }
}

TEST_F(GainProcessingTest, GainClipping) {
    ProcessedWav result = processWavFile(testFile, 10.0);
    
    ASSERT_EQ(result.originalSamples.size(), result.processedSamples.size());
    
    // With high gain, samples should be clipped at int16 limits
    constexpr int16_t maxVal = std::numeric_limits<int16_t>::max();
    constexpr int16_t minVal = std::numeric_limits<int16_t>::min();
    
    for (size_t i = 0; i < result.processedSamples.size(); ++i) {
        EXPECT_GE(result.processedSamples[i], minVal);
        EXPECT_LE(result.processedSamples[i], maxVal);
    }
}

TEST_F(GainProcessingTest, GainZero) {
    ProcessedWav result = processWavFile(testFile, 0.0);
    
    ASSERT_EQ(result.originalSamples.size(), result.processedSamples.size());
    
    // With gain 0, all samples should be zero (or very close due to rounding)
    for (size_t i = 0; i < result.processedSamples.size(); ++i) {
        EXPECT_EQ(result.processedSamples[i], 0)
            << "Sample " << i << " should be 0 with gain 0";
    }
}

TEST_F(GainProcessingTest, OutputFileCreated) {
    ProcessedWav result = processWavFile(testFile, 0.8);
    
    std::ifstream outFile(result.processedPath, std::ios::binary);
    EXPECT_TRUE(outFile.is_open()) << "Output file was not created: " << result.processedPath;
    
    if (outFile.is_open()) {
        outFile.seekg(0, std::ios::end);
        size_t fileSize = outFile.tellg();
        EXPECT_GT(fileSize, 44) << "Output file is suspiciously small";
    }
}
