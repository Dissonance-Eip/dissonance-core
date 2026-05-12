#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "core/Errors.hpp"
#include "utils/WavParser.hpp"

namespace {

void writeMinimalWav(const std::string &path, uint16_t numChannels, uint32_t sampleRate,
                     uint16_t bitsPerSample, const std::vector<int16_t> &samples) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    const uint32_t subchunk2Size = static_cast<uint32_t>(samples.size() * sizeof(int16_t));
    const uint32_t chunkSize = 36 + subchunk2Size;
    const uint32_t subchunk1Size = 16;
    const uint16_t audioFormat = 1;
    const uint16_t blockAlign = static_cast<uint16_t>(numChannels * (bitsPerSample / 8));
    const uint32_t byteRate = sampleRate * blockAlign;

    f.write("RIFF", 4);
    f.write(reinterpret_cast<const char *>(&chunkSize), 4);
    f.write("WAVE", 4);
    f.write("fmt ", 4);
    f.write(reinterpret_cast<const char *>(&subchunk1Size), 4);
    f.write(reinterpret_cast<const char *>(&audioFormat), 2);
    f.write(reinterpret_cast<const char *>(&numChannels), 2);
    f.write(reinterpret_cast<const char *>(&sampleRate), 4);
    f.write(reinterpret_cast<const char *>(&byteRate), 4);
    f.write(reinterpret_cast<const char *>(&blockAlign), 2);
    f.write(reinterpret_cast<const char *>(&bitsPerSample), 2);
    f.write("data", 4);
    f.write(reinterpret_cast<const char *>(&subchunk2Size), 4);
    for (int16_t s : samples)
        f.write(reinterpret_cast<const char *>(&s), 2);
}

std::string tmpPath(const std::string &name) {
    return (std::filesystem::temp_directory_path() / name).string();
}

} // namespace

TEST(WavRoundtripTest, MonoFieldsMatch) {
    const std::string path = tmpPath("rt_mono.wav");
    std::vector<int16_t> samples = {0, 16383, -16384, 32767, -32768};
    writeMinimalWav(path, 1, 44100, 16, samples);

    std::ifstream f(path, std::ios::binary);
    Parser p = Parser::fromFile(f);

    EXPECT_EQ(p.getNumChannels(), 1);
    EXPECT_EQ(p.getSampleRate(), 44100u);
    EXPECT_EQ(p.getBitsPerSample(), 16);
    EXPECT_EQ(p.getAudioFormat(), 1);
    ASSERT_EQ(p.getAudioData().size(), samples.size());

    std::filesystem::remove(path);
}

TEST(WavRoundtripTest, StereoSampleCountCorrect) {
    const std::string path = tmpPath("rt_stereo.wav");
    // 4 frames × 2 channels = 8 int16 samples
    std::vector<int16_t> samples = {100, -100, 200, -200, 300, -300, 400, -400};
    writeMinimalWav(path, 2, 48000, 16, samples);

    std::ifstream f(path, std::ios::binary);
    Parser p = Parser::fromFile(f);

    EXPECT_EQ(p.getNumChannels(), 2);
    EXPECT_EQ(p.getSampleRate(), 48000u);
    ASSERT_EQ(p.getAudioData().size(), samples.size());

    std::filesystem::remove(path);
}

TEST(WavRoundtripTest, FloatNormalisationInRange) {
    const std::string path = tmpPath("rt_norm.wav");
    std::vector<int16_t> samples = {32767, -32768, 0};
    writeMinimalWav(path, 1, 44100, 16, samples);

    std::ifstream f(path, std::ios::binary);
    Parser p = Parser::fromFile(f);

    for (float v : p.getAudioData()) {
        EXPECT_GE(v, -1.0f);
        EXPECT_LE(v, 1.0f);
    }

    std::filesystem::remove(path);
}

TEST(WavRoundtripTest, TruncatedFileThrows) {
    const std::string path = tmpPath("rt_trunc.wav");
    {
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        f.write("RIFF", 4); // deliberately incomplete
    }
    std::ifstream f(path, std::ios::binary);
    EXPECT_THROW(Parser::fromFile(f), dissonance::WavFormatError);
    std::filesystem::remove(path);
}

TEST(WavRoundtripTest, WrongMagicThrows) {
    const std::string path = tmpPath("rt_magic.wav");
    {
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        f.write("BIFF", 4);
        uint32_t dummy = 0;
        f.write(reinterpret_cast<const char *>(&dummy), 4);
        f.write("WAVE", 4);
    }
    std::ifstream f(path, std::ios::binary);
    EXPECT_THROW(Parser::fromFile(f), dissonance::WavFormatError);
    std::filesystem::remove(path);
}
