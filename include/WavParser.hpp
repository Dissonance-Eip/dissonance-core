#ifndef WAVPARSER_H
    #define WAVPARSER_H

#include <cstdint>
#include <fstream>
#include <string>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <unordered_map>

class Parser {
public:
    Parser() = default;

    void readFromFile(std::ifstream& file) {
        if (!file.is_open()) {
            throw std::runtime_error("File not open");
        }

        readString(file, riff, 4);
        readData(file, chunkSize);
        readString(file, wave, 4);
        readString(file, fmt, 4);
        readData(file, subchunk1Size);
        readData(file, audioFormat);
        readData(file, numChannels);
        readData(file, sampleRate);
        readData(file, byteRate);
        readData(file, blockAlign);
        readData(file, bitsPerSample);

        while (true) {
            readString(file, data, 4);
            uint32_t chunkSize;
            readData(file, chunkSize);

            if (data == "data") {
                subchunk2Size = chunkSize;
                audioData.resize(subchunk2Size);

                if (bitsPerSample == 16) {
                    std::vector<int16_t> tempData(subchunk2Size / 2);
                    if (!file.read(reinterpret_cast<char*>(tempData.data()), subchunk2Size)) {
                        throw std::runtime_error("Failed to read audio data");
                    }
                    audioData.assign(tempData.begin(), tempData.end());
                }
                break;
            }
            if (data == "LIST") {
                std::vector<char> chunkData(chunkSize);
                if (!file.read(chunkData.data(), chunkSize)) {
                    throw std::runtime_error("Failed to read LIST chunk data");
                }
                otherChunks[data] = std::move(chunkData);
            } else {
                std::vector<char> chunkData(chunkSize);
                if (!file.read(chunkData.data(), chunkSize)) {
                    throw std::runtime_error("Failed to read chunk data");
                }
                otherChunks[data] = std::move(chunkData);
            }
        }
    }

    // Getters
    [[nodiscard]] std::string getRiff() const { return riff; }
    [[nodiscard]] uint32_t getChunkSize() const { return chunkSize; }
    [[nodiscard]] std::string getWave() const { return wave; }
    [[nodiscard]] std::string getFmt() const { return fmt; }
    [[nodiscard]] uint32_t getSubchunk1Size() const { return subchunk1Size; }
    [[nodiscard]] uint16_t getAudioFormat() const { return audioFormat; }
    [[nodiscard]] uint16_t getNumChannels() const { return numChannels; }
    [[nodiscard]] uint32_t getSampleRate() const { return sampleRate; }
    [[nodiscard]] uint32_t getByteRate() const { return byteRate; }
    [[nodiscard]] uint16_t getBlockAlign() const { return blockAlign; }
    [[nodiscard]] uint16_t getBitsPerSample() const { return bitsPerSample; }
    [[nodiscard]] std::string getData() const { return data; }
    [[nodiscard]] uint32_t getSubchunk2Size() const { return subchunk2Size; }
    [[nodiscard]] const std::vector<int16_t>& getAudioData() const { return audioData; }
    [[nodiscard]] const std::unordered_map<std::string, std::vector<char>>& getOtherChunks() const { return otherChunks; }

private:
    static void readString(std::ifstream& file, std::string& field, const size_t size) {
        field.resize(size);
        if (!file.read(&field[0], static_cast<std::streamsize>(size))) {
            throw std::runtime_error("Failed to read string field");
        }
    }

    template <typename T>
    static void readData(std::ifstream& file, T& field) {
        if (!file.read(reinterpret_cast<char*>(&field), sizeof(field))) {
            throw std::runtime_error("Failed to read data field");
        }
    }

    std::string riff;                     // "RIFF"
    uint32_t chunkSize{0};                // Size of the entire file in bytes minus 8 bytes
    std::string wave;                     // "WAVE"
    std::string fmt;                      // "fmt "
    uint32_t subchunk1Size{0};            // Size of the fmt chunk
    uint16_t audioFormat{0};              // Audio format (1 for PCM)
    uint16_t numChannels{0};              // Number of channels
    uint32_t sampleRate{0};               // Sample rate
    uint32_t byteRate{0};                 // Byte rate
    uint16_t blockAlign{0};               // Block align
    uint16_t bitsPerSample{0};            // Bits per sample
    std::string data;                     // "data"
    uint32_t subchunk2Size{0};            // Size of the data chunk
    std::vector<int16_t> audioData;       // Audio data
    std::unordered_map<std::string, std::vector<char>> otherChunks; // Other chunks (e.g., "LIST")
};

#endif //WAVPARSER_H