#include "WavProcessor.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "GainProcessor.hpp"

namespace fs = std::filesystem;

namespace {

std::string makeOutputPath(const std::string& inputPath, const std::string& customPath) {
    if (!customPath.empty()) {
        return customPath;
    }

    fs::path inPath(inputPath);
    fs::path parent = inPath.parent_path();
    std::string stem = inPath.stem().string();
    std::string ext = inPath.extension().string();
    if (ext.empty()) {
        ext = ".wav";
    }

    fs::path out = parent / (stem + "-processed" + ext);
    return out.string();
}

void writeWavFile(const Parser& parser, const std::vector<int16_t>& samples, const std::string& outputPath) {
    if (parser.getBitsPerSample() != 16) {
        throw std::runtime_error("Only 16-bit PCM WAV is supported for writing");
    }

    std::ofstream out(outputPath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open output file: " + outputPath);
    }

    const uint32_t subchunk2Size = static_cast<uint32_t>(samples.size() * sizeof(int16_t));
    const uint32_t chunkSize = 36 + subchunk2Size; // PCM fmt chunk (16 bytes) + data

    out.write("RIFF", 4);
    out.write(reinterpret_cast<const char*>(&chunkSize), sizeof(chunkSize));
    out.write("WAVE", 4);

    // fmt chunk
    const uint32_t subchunk1Size = parser.getSubchunk1Size();
    out.write("fmt ", 4);
    out.write(reinterpret_cast<const char*>(&subchunk1Size), sizeof(subchunk1Size));
    const uint16_t audioFormat = parser.getAudioFormat();
    const uint16_t numChannels = parser.getNumChannels();
    const uint32_t sampleRate = parser.getSampleRate();
    const uint32_t byteRate = parser.getByteRate();
    const uint16_t blockAlign = parser.getBlockAlign();
    const uint16_t bitsPerSample = parser.getBitsPerSample();
    out.write(reinterpret_cast<const char*>(&audioFormat), sizeof(audioFormat));
    out.write(reinterpret_cast<const char*>(&numChannels), sizeof(numChannels));
    out.write(reinterpret_cast<const char*>(&sampleRate), sizeof(sampleRate));
    out.write(reinterpret_cast<const char*>(&byteRate), sizeof(byteRate));
    out.write(reinterpret_cast<const char*>(&blockAlign), sizeof(blockAlign));
    out.write(reinterpret_cast<const char*>(&bitsPerSample), sizeof(bitsPerSample));

    // data chunk
    out.write("data", 4);
    out.write(reinterpret_cast<const char*>(&subchunk2Size), sizeof(subchunk2Size));
    out.write(reinterpret_cast<const char*>(samples.data()), static_cast<std::streamsize>(samples.size() * sizeof(int16_t)));
}

} // namespace

ProcessedWav processWavFile(const std::string& inputPath, double gain, const std::string& outputPath) {
    Parser parser;
    std::ifstream file(inputPath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + inputPath);
    }
    parser.readFromFile(file);

    ProcessedWav result;
    result.parser = parser;
    result.otherChunks = parser.getOtherChunks();
    result.originalSamples = parser.getAudioData();
    result.processedSamples = result.originalSamples;

    GainProcessor gainProcessor(gain);
    gainProcessor.apply(result.processedSamples);
    result.processedPath = makeOutputPath(inputPath, outputPath);
    writeWavFile(parser, result.processedSamples, result.processedPath);

    result.metadataText = formatMetadataText(parser);
    result.waveformText = renderWaveformASCII(result.originalSamples);

    if (auto it = result.otherChunks.find("LIST"); it != result.otherChunks.end()) {
        result.listTags = parseListChunk(it->second);
    }

    return result;
}
