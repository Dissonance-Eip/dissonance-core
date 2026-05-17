/**
 * @file WavProcessor.cpp
 * @brief Top-level audio processing entry point.
 *
 * processWavFile() reads a WAV, runs it through the Pipeline
 * (WindowedFFTStage → GainStage), writes the output, and returns a
 * ProcessedWav with metadata and statistics.
 */

#include "audio/WavProcessor.hpp"
#include "core/Errors.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>

#include "audio/GainStage.hpp"
#include "audio/Pipeline.hpp"
#include "audio/WindowedFFTStage.hpp"

namespace fs = std::filesystem;

namespace {

std::string makeOutputPath(const std::string &inputPath, const std::string &customPath) {
    if (!customPath.empty())
        return customPath;
    fs::path in(inputPath);
    std::string ext = in.extension().string();
    if (ext.empty())
        ext = ".wav";
    return (in.parent_path() / (in.stem().string() + "-processed" + ext)).string();
}

void writeWavFile(const Parser &parser, const std::vector<float> &samples,
                  const std::string &outputPath) {
    std::ofstream out(outputPath, std::ios::binary | std::ios::trunc);
    if (!out.is_open())
        throw dissonance::WavFormatError("Failed to open output file: " + outputPath);

    const uint32_t subchunk2Size = static_cast<uint32_t>(samples.size() * sizeof(int16_t));
    const uint32_t chunkSize = 36 + subchunk2Size;

    out.write("RIFF", 4);
    out.write(reinterpret_cast<const char *>(&chunkSize), sizeof(chunkSize));
    out.write("WAVE", 4);

    const uint32_t subchunk1Size = 16;
    out.write("fmt ", 4);
    out.write(reinterpret_cast<const char *>(&subchunk1Size), sizeof(subchunk1Size));
    const uint16_t audioFormat = 1;
    const uint16_t numChannels = parser.getNumChannels();
    const uint32_t sampleRate = parser.getSampleRate();
    const uint16_t bitsPerSample = 16;
    const uint16_t blockAlign = static_cast<uint16_t>(numChannels * (bitsPerSample / 8));
    const uint32_t byteRate = sampleRate * blockAlign;
    out.write(reinterpret_cast<const char *>(&audioFormat), sizeof(audioFormat));
    out.write(reinterpret_cast<const char *>(&numChannels), sizeof(numChannels));
    out.write(reinterpret_cast<const char *>(&sampleRate), sizeof(sampleRate));
    out.write(reinterpret_cast<const char *>(&byteRate), sizeof(byteRate));
    out.write(reinterpret_cast<const char *>(&blockAlign), sizeof(blockAlign));
    out.write(reinterpret_cast<const char *>(&bitsPerSample), sizeof(bitsPerSample));

    out.write("data", 4);
    out.write(reinterpret_cast<const char *>(&subchunk2Size), sizeof(subchunk2Size));

    for (float s : samples) {
        const int16_t pcm = static_cast<int16_t>(std::clamp(s, -1.0f, 1.0f) * 32767.0f);
        out.write(reinterpret_cast<const char *>(&pcm), sizeof(pcm));
    }
}

} // namespace

ProcessedWav processWavFile(const std::string &inputPath, const ProcessingOptions &opts) {
    std::ifstream file(inputPath, std::ios::binary);
    if (!file.is_open())
        throw dissonance::WavFormatError("Failed to open file: " + inputPath);

    Parser parser = Parser::fromFile(file);

    ProcessedWav result;
    result.parser = parser;
    result.otherChunks = parser.getOtherChunks();
    result.originalSamples = parser.getAudioData();
    result.processedSamples = result.originalSamples;

    auto fftOwned = std::make_unique<WindowedFFTStage>(2048, 0.25f, opts.progressCallback);
    const WindowedFFTStage *fftStage = fftOwned.get();

    Pipeline pipeline;
    pipeline.addStage(std::make_unique<GainStage>(opts.gain));
    pipeline.addStage(std::move(fftOwned));
    pipeline.run(result.processedSamples, parser.getNumChannels());

    if (fftStage->framesProcessed() > 0)
        result.fftReport = {true, fftStage->framesProcessed(), fftStage->bins(),
                            fftStage->cutoffBin()};

    result.processedPath = makeOutputPath(inputPath, opts.outputPath);
    writeWavFile(parser, result.processedSamples, result.processedPath);

    result.metadataText = formatMetadataText(parser);
    result.waveformText = renderWaveformASCII(result.originalSamples);

    if (auto it = result.otherChunks.find("LIST"); it != result.otherChunks.end())
        result.listTags = parseListChunk(it->second);

    return result;
}
