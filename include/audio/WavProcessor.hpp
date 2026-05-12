#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "../utils/WavParser.hpp"
#include "../utils/WavUtils.hpp"

struct ProcessedWav {
    Parser parser; // parsed header/info
    std::unordered_map<std::string, std::vector<char>> otherChunks;
    std::vector<int16_t> originalSamples;
    std::vector<int16_t> processedSamples;
    ListTags listTags;
    std::string metadataText;
    std::string waveformText;
    std::string processedPath;
    bool fftApplied = false;
    size_t fftFramesProcessed = 0;
    size_t fftBins = 0;
    size_t fftCutoffBin = 0;
};

ProcessedWav processWavFile(const std::string &inputPath, double gain = 0.8,
                            const std::string &outputPath = "");
