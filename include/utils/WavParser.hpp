#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/Errors.hpp"

/**
 * @brief RIFF/WAV file parser.
 *
 * Reads and validates a WAV file header and optionally decodes the audio
 * data into normalised floats in [-1, 1]. Supports PCM 8/16/24/32-bit and
 * IEEE float 32/64-bit formats. Unknown chunks are preserved verbatim.
 *
 * @throws dissonance::WavFormatError on malformed or unsupported input.
 */
class Parser {
  public:
    Parser() = default;

    static Parser fromFile(std::ifstream &file, bool readAudioData = true);
    void readFromFile(std::ifstream &file, bool readAudioData = true);

    [[nodiscard]] const std::string &getRiff() const { return riff; }
    [[nodiscard]] uint32_t getChunkSize() const { return chunkSize; }
    [[nodiscard]] const std::string &getWave() const { return wave; }
    [[nodiscard]] const std::string &getFmt() const { return fmt; }
    [[nodiscard]] uint32_t getSubchunk1Size() const { return subchunk1Size; }
    [[nodiscard]] uint16_t getAudioFormat() const { return audioFormat; }
    [[nodiscard]] uint16_t getNumChannels() const { return numChannels; }
    [[nodiscard]] uint32_t getSampleRate() const { return sampleRate; }
    [[nodiscard]] uint32_t getByteRate() const { return byteRate; }
    [[nodiscard]] uint16_t getBlockAlign() const { return blockAlign; }
    [[nodiscard]] uint16_t getBitsPerSample() const { return bitsPerSample; }
    [[nodiscard]] const std::string &getData() const { return data; }
    [[nodiscard]] uint32_t getSubchunk2Size() const { return subchunk2Size; }
    [[nodiscard]] const std::vector<float> &getAudioData() const { return audioData; }
    [[nodiscard]] const std::unordered_map<std::string, std::vector<char>> &getOtherChunks() const {
        return otherChunks;
    }

  private:
    std::string riff;
    uint32_t chunkSize{0};
    std::string wave;
    std::string fmt;
    uint32_t subchunk1Size{0};
    uint16_t audioFormat{0};
    uint16_t numChannels{0};
    uint32_t sampleRate{0};
    uint32_t byteRate{0};
    uint16_t blockAlign{0};
    uint16_t bitsPerSample{0};
    std::string data;
    uint32_t subchunk2Size{0};
    std::vector<float> audioData;
    std::unordered_map<std::string, std::vector<char>> otherChunks;
};
