#ifndef WAVPROCESSOR_H
#define WAVPROCESSOR_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include "WavParser.hpp"
#include "WavUtils.hpp"

struct ProcessedWav {
    Parser parser; // parsed header/info
    std::unordered_map<std::string, std::vector<char>> otherChunks;
    std::vector<int16_t> originalSamples;
    std::vector<int16_t> processedSamples;
    ListTags listTags;
    std::string metadataText;
    std::string waveformText;
    std::string processedPath;
};

ProcessedWav processWavFile(const std::string& inputPath, double gain = 0.8);

#endif // WAVPROCESSOR_H
