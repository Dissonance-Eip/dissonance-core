#ifndef WAVPARSER_H
#define WAVPARSER_H

#include <cstdint>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <string>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <unordered_map>

class Parser {
  public:
    Parser() = default;

    void readFromFile(std::ifstream &file, bool readAudioData = true) {
        if (!file.is_open()) {
            throw std::runtime_error("File not open");
        }

        readString(file, riff, 4);
        readData(file, chunkSize);
        readString(file, wave, 4);

        if (riff != "RIFF" || wave != "WAVE") {
            throw std::runtime_error("Not a valid RIFF/WAVE file");
        }

        auto skipPaddingByteIfNeeded = [&](uint32_t size) {
            if (size % 2 != 0) {
                file.seekg(1, std::ios::cur);
            }
        };

        bool sawFmt = false;

        while (true) {
            std::string chunkId;
            readString(file, chunkId, 4);
            uint32_t currentChunkSize = 0;
            readData(file, currentChunkSize);

            if (chunkId == "fmt ") {
                fmt = chunkId;
                subchunk1Size = currentChunkSize;

                // Base fmt (PCM) is 16 bytes.
                readData(file, audioFormat);
                readData(file, numChannels);
                readData(file, sampleRate);
                readData(file, byteRate);
                readData(file, blockAlign);
                readData(file, bitsPerSample);

                if (currentChunkSize > 16) {
                    file.seekg(static_cast<std::streamoff>(currentChunkSize - 16), std::ios::cur);
                }

                sawFmt = true;
                skipPaddingByteIfNeeded(currentChunkSize);
                continue;
            }

            if (chunkId == "data") {
                if (!sawFmt) {
                    throw std::runtime_error("WAV missing fmt chunk before data");
                }

                data = chunkId;
                subchunk2Size = currentChunkSize;

                const uint16_t bytesPerSample = static_cast<uint16_t>(bitsPerSample / 8);
                if (bytesPerSample == 0) {
                    throw std::runtime_error("Invalid bitsPerSample in WAV header");
                }

                if (currentChunkSize % bytesPerSample != 0) {
                    throw std::runtime_error("Corrupt WAV data chunk size");
                }

                const size_t sampleCount = static_cast<size_t>(currentChunkSize / bytesPerSample);
                audioData.clear();
                if (readAudioData) {
                    audioData.resize(sampleCount);
                }

                auto clamp16 = [](int value) {
                    if (value < std::numeric_limits<int16_t>::min())
                        return std::numeric_limits<int16_t>::min();
                    if (value > std::numeric_limits<int16_t>::max())
                        return std::numeric_limits<int16_t>::max();
                    return static_cast<int16_t>(value);
                };

                if (audioFormat == 1) {
                    // PCM
                    if (bitsPerSample == 16) {
                        if (readAudioData) {
                            if (!file.read(reinterpret_cast<char *>(audioData.data()),
                                           static_cast<std::streamsize>(currentChunkSize))) {
                                throw std::runtime_error("Failed to read audio data");
                            }
                        } else {
                            file.seekg(static_cast<std::streamoff>(currentChunkSize),
                                       std::ios::cur);
                        }
                    } else if (bitsPerSample == 8) {
                        std::vector<uint8_t> buf(currentChunkSize);
                        if (!file.read(reinterpret_cast<char *>(buf.data()),
                                       static_cast<std::streamsize>(currentChunkSize))) {
                            throw std::runtime_error("Failed to read audio data");
                        }
                        if (readAudioData) {
                            audioData.resize(sampleCount);
                            for (size_t i = 0; i < sampleCount; ++i) {
                                const int centered = static_cast<int>(buf[i]) - 128;
                                audioData[i] = clamp16(centered << 8);
                            }
                        }
                    } else if (bitsPerSample == 24) {
                        std::vector<uint8_t> buf(currentChunkSize);
                        if (!file.read(reinterpret_cast<char *>(buf.data()),
                                       static_cast<std::streamsize>(currentChunkSize))) {
                            throw std::runtime_error("Failed to read audio data");
                        }
                        if (readAudioData) {
                            audioData.resize(sampleCount);
                            for (size_t i = 0, o = 0; o < sampleCount; i += 3, ++o) {
                                int32_t v = static_cast<int32_t>(buf[i]) |
                                            (static_cast<int32_t>(buf[i + 1]) << 8) |
                                            (static_cast<int32_t>(buf[i + 2]) << 16);
                                if (v & 0x00800000) {
                                    v |= ~0x00FFFFFF;
                                }
                                audioData[o] = clamp16(static_cast<int>(v >> 8));
                            }
                        }
                    } else if (bitsPerSample == 32) {
                        std::vector<int32_t> buf(sampleCount);
                        if (!file.read(reinterpret_cast<char *>(buf.data()),
                                       static_cast<std::streamsize>(currentChunkSize))) {
                            throw std::runtime_error("Failed to read audio data");
                        }
                        if (readAudioData) {
                            audioData.resize(sampleCount);
                            for (size_t i = 0; i < sampleCount; ++i) {
                                audioData[i] = clamp16(static_cast<int>(buf[i] >> 16));
                            }
                        }
                    } else {
                        throw std::runtime_error("Unsupported PCM bitsPerSample: " +
                                                 std::to_string(bitsPerSample));
                    }
                } else if (audioFormat == 3) {
                    // IEEE float
                    if (bitsPerSample == 32) {
                        std::vector<float> buf(sampleCount);
                        if (!file.read(reinterpret_cast<char *>(buf.data()),
                                       static_cast<std::streamsize>(currentChunkSize))) {
                            throw std::runtime_error("Failed to read audio data");
                        }
                        if (readAudioData) {
                            audioData.resize(sampleCount);
                            for (size_t i = 0; i < sampleCount; ++i) {
                                const double s = static_cast<double>(buf[i]);
                                const double clipped = std::clamp(s, -1.0, 1.0);
                                const int scaled = static_cast<int>(std::lround(clipped * 32767.0));
                                audioData[i] = clamp16(scaled);
                            }
                        }
                    } else if (bitsPerSample == 64) {
                        std::vector<double> buf(sampleCount);
                        if (!file.read(reinterpret_cast<char *>(buf.data()),
                                       static_cast<std::streamsize>(currentChunkSize))) {
                            throw std::runtime_error("Failed to read audio data");
                        }
                        if (readAudioData) {
                            audioData.resize(sampleCount);
                            for (size_t i = 0; i < sampleCount; ++i) {
                                const double clipped = std::clamp(buf[i], -1.0, 1.0);
                                const int scaled = static_cast<int>(std::lround(clipped * 32767.0));
                                audioData[i] = clamp16(scaled);
                            }
                        }
                    } else {
                        throw std::runtime_error("Unsupported float bitsPerSample: " +
                                                 std::to_string(bitsPerSample));
                    }
                } else {
                    throw std::runtime_error("Unsupported WAV audioFormat: " +
                                             std::to_string(audioFormat));
                }

                skipPaddingByteIfNeeded(currentChunkSize);
                break;
            }

            // Any other chunk
            std::vector<char> chunkData(currentChunkSize);
            if (!file.read(chunkData.data(), static_cast<std::streamsize>(currentChunkSize))) {
                throw std::runtime_error("Failed to read chunk data");
            }
            otherChunks[chunkId] = std::move(chunkData);
            skipPaddingByteIfNeeded(currentChunkSize);
        }
    }

    // Getters
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
    [[nodiscard]] const std::vector<int16_t> &getAudioData() const { return audioData; }
    [[nodiscard]] const std::unordered_map<std::string, std::vector<char>> &getOtherChunks() const {
        return otherChunks;
    }

  private:
    static void readString(std::ifstream &file, std::string &field, const size_t size) {
        field.resize(size);
        if (!file.read(&field[0], static_cast<std::streamsize>(size))) {
            throw std::runtime_error("Failed to read string field");
        }
    }

    template <typename T> static void readData(std::ifstream &file, T &field) {
        if (!file.read(reinterpret_cast<char *>(&field), sizeof(field))) {
            throw std::runtime_error("Failed to read data field");
        }
    }

    std::string riff;               // "RIFF"
    uint32_t chunkSize{0};          // Size of the entire file in bytes minus 8 bytes
    std::string wave;               // "WAVE"
    std::string fmt;                // "fmt "
    uint32_t subchunk1Size{0};      // Size of the fmt chunk
    uint16_t audioFormat{0};        // Audio format (1 for PCM)
    uint16_t numChannels{0};        // Number of channels
    uint32_t sampleRate{0};         // Sample rate
    uint32_t byteRate{0};           // Byte rate
    uint16_t blockAlign{0};         // Block align
    uint16_t bitsPerSample{0};      // Bits per sample
    std::string data;               // "data"
    uint32_t subchunk2Size{0};      // Size of the data chunk
    std::vector<int16_t> audioData; // Audio data
    std::unordered_map<std::string, std::vector<char>> otherChunks; // Other chunks (e.g., "LIST")
};

#endif // WAVPARSER_H
