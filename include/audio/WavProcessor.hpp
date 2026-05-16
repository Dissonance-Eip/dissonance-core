#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../utils/WavParser.hpp"
#include "../utils/WavUtils.hpp"

struct FFTReport {
    bool applied = false;
    size_t framesProcessed = 0;
    size_t bins = 0;
    size_t cutoffBin = 0;
};

struct ProcessingOptions {
    double gain = 0.8;
    std::string outputPath;
    std::function<void(float)> progressCallback;
};

struct ProcessedWav {
    Parser parser;
    std::unordered_map<std::string, std::vector<char>> otherChunks;
    std::vector<float> originalSamples;
    std::vector<float> processedSamples;
    ListTags listTags;
    std::string metadataText;
    std::string waveformText;
    std::string processedPath;
    FFTReport fftReport;
};

ProcessedWav processWavFile(const std::string &inputPath, const ProcessingOptions &opts = {});
