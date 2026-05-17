/**
 * @file WavParser.cpp
 * @brief RIFF/WAV file parser — reads header fields and decodes audio samples.
 *
 * Supports PCM 8/16/24/32-bit and IEEE float 32/64-bit formats. All decoded
 * samples are normalised to [-1, 1]. Unknown chunks are stored verbatim for
 * round-trip preservation.
 */

#include "utils/WavParser.hpp"

#include <algorithm>
#include <cmath>

namespace {

void readString(std::ifstream &file, std::string &field, size_t size) {
    field.resize(size);
    if (!file.read(&field[0], static_cast<std::streamsize>(size))) {
        throw dissonance::WavFormatError("Failed to read string field");
    }
}

template <typename T> void readData(std::ifstream &file, T &field) {
    if (!file.read(reinterpret_cast<char *>(&field), sizeof(field))) {
        throw dissonance::WavFormatError("Failed to read data field");
    }
}

} // namespace

void Parser::readFromFile(std::ifstream &file, bool readAudioData) {
    if (!file.is_open()) {
        throw dissonance::WavFormatError("File not open");
    }

    readString(file, riff, 4);
    readData(file, chunkSize);
    readString(file, wave, 4);

    if (riff != "RIFF" || wave != "WAVE") {
        throw dissonance::WavFormatError("Not a valid RIFF/WAVE file");
    }

    auto skipPadding = [&](uint32_t size) {
        if (size % 2 != 0)
            file.seekg(1, std::ios::cur);
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

            readData(file, audioFormat);
            readData(file, numChannels);
            readData(file, sampleRate);
            readData(file, byteRate);
            readData(file, blockAlign);
            readData(file, bitsPerSample);

            if (currentChunkSize > 16)
                file.seekg(static_cast<std::streamoff>(currentChunkSize - 16), std::ios::cur);

            sawFmt = true;
            skipPadding(currentChunkSize);
            continue;
        }

        if (chunkId == "data") {
            if (!sawFmt)
                throw dissonance::WavFormatError("WAV missing fmt chunk before data");

            data = chunkId;
            subchunk2Size = currentChunkSize;

            const uint16_t bytesPerSample = static_cast<uint16_t>(bitsPerSample / 8);
            if (bytesPerSample == 0)
                throw dissonance::WavFormatError("Invalid bitsPerSample in WAV header");

            if (currentChunkSize % bytesPerSample != 0)
                throw dissonance::WavFormatError("Corrupt WAV data chunk size");

            const size_t sampleCount = static_cast<size_t>(currentChunkSize / bytesPerSample);
            audioData.clear();

            if (audioFormat == 1) {
                if (bitsPerSample == 16) {
                    if (readAudioData) {
                        std::vector<int16_t> buf(sampleCount);
                        if (!file.read(reinterpret_cast<char *>(buf.data()),
                                       static_cast<std::streamsize>(currentChunkSize)))
                            throw dissonance::WavFormatError("Failed to read audio data");
                        audioData.resize(sampleCount);
                        for (size_t i = 0; i < sampleCount; ++i)
                            audioData[i] = static_cast<float>(buf[i]) / 32768.0f;
                    } else {
                        file.seekg(static_cast<std::streamoff>(currentChunkSize), std::ios::cur);
                    }
                } else if (bitsPerSample == 8) {
                    std::vector<uint8_t> buf(currentChunkSize);
                    if (!file.read(reinterpret_cast<char *>(buf.data()),
                                   static_cast<std::streamsize>(currentChunkSize)))
                        throw dissonance::WavFormatError("Failed to read audio data");
                    if (readAudioData) {
                        audioData.resize(sampleCount);
                        for (size_t i = 0; i < sampleCount; ++i)
                            audioData[i] = (static_cast<float>(buf[i]) - 128.0f) / 128.0f;
                    }
                } else if (bitsPerSample == 24) {
                    std::vector<uint8_t> buf(currentChunkSize);
                    if (!file.read(reinterpret_cast<char *>(buf.data()),
                                   static_cast<std::streamsize>(currentChunkSize)))
                        throw dissonance::WavFormatError("Failed to read audio data");
                    if (readAudioData) {
                        audioData.resize(sampleCount);
                        for (size_t i = 0, o = 0; o < sampleCount; i += 3, ++o) {
                            int32_t v = static_cast<int32_t>(buf[i]) |
                                        (static_cast<int32_t>(buf[i + 1]) << 8) |
                                        (static_cast<int32_t>(buf[i + 2]) << 16);
                            if (v & 0x00800000)
                                v |= ~0x00FFFFFF;
                            audioData[o] = static_cast<float>(v) / 8388608.0f;
                        }
                    }
                } else if (bitsPerSample == 32) {
                    std::vector<int32_t> buf(sampleCount);
                    if (!file.read(reinterpret_cast<char *>(buf.data()),
                                   static_cast<std::streamsize>(currentChunkSize)))
                        throw dissonance::WavFormatError("Failed to read audio data");
                    if (readAudioData) {
                        audioData.resize(sampleCount);
                        for (size_t i = 0; i < sampleCount; ++i)
                            audioData[i] = static_cast<float>(buf[i]) / 2147483648.0f;
                    }
                } else {
                    throw dissonance::WavFormatError("Unsupported PCM bitsPerSample: " +
                                                     std::to_string(bitsPerSample));
                }
            } else if (audioFormat == 3) {
                if (bitsPerSample == 32) {
                    if (readAudioData) {
                        audioData.resize(sampleCount);
                        if (!file.read(reinterpret_cast<char *>(audioData.data()),
                                       static_cast<std::streamsize>(currentChunkSize)))
                            throw dissonance::WavFormatError("Failed to read audio data");
                        for (auto &s : audioData)
                            s = std::clamp(s, -1.0f, 1.0f);
                    } else {
                        file.seekg(static_cast<std::streamoff>(currentChunkSize), std::ios::cur);
                    }
                } else if (bitsPerSample == 64) {
                    std::vector<double> buf(sampleCount);
                    if (!file.read(reinterpret_cast<char *>(buf.data()),
                                   static_cast<std::streamsize>(currentChunkSize)))
                        throw dissonance::WavFormatError("Failed to read audio data");
                    if (readAudioData) {
                        audioData.resize(sampleCount);
                        for (size_t i = 0; i < sampleCount; ++i)
                            audioData[i] = static_cast<float>(std::clamp(buf[i], -1.0, 1.0));
                    }
                } else {
                    throw dissonance::WavFormatError("Unsupported float bitsPerSample: " +
                                                     std::to_string(bitsPerSample));
                }
            } else {
                throw dissonance::WavFormatError("Unsupported WAV audioFormat: " +
                                                 std::to_string(audioFormat));
            }

            skipPadding(currentChunkSize);
            break;
        }

        std::vector<char> chunkData(currentChunkSize);
        if (!file.read(chunkData.data(), static_cast<std::streamsize>(currentChunkSize)))
            throw dissonance::WavFormatError("Failed to read chunk data");
        otherChunks[chunkId] = std::move(chunkData);
        skipPadding(currentChunkSize);
    }
}

Parser Parser::fromFile(std::ifstream &file, bool readAudioData) {
    Parser p;
    p.readFromFile(file, readAudioData);
    return p;
}
