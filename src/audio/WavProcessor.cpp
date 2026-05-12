#include "audio/WavProcessor.hpp"
#include "core/Errors.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "audio/FFTProcessor.hpp"
#include "audio/GainProcessor.hpp"
#include "audio/WindowFunctions.hpp"

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

ProcessedWav processWavFile(const std::string &inputPath, ProcessingOptions opts) {
    std::ifstream file(inputPath, std::ios::binary);
    if (!file.is_open())
        throw dissonance::WavFormatError("Failed to open file: " + inputPath);

    Parser parser = Parser::fromFile(file);

    ProcessedWav result;
    result.parser = parser;
    result.otherChunks = parser.getOtherChunks();
    result.originalSamples = parser.getAudioData();
    result.processedSamples = result.originalSamples;

    GainProcessor gainProcessor(opts.gain);
    gainProcessor.apply(result.processedSamples);

    result.processedPath = makeOutputPath(inputPath, opts.outputPath);
    writeWavFile(parser, result.processedSamples, result.processedPath);

    result.metadataText = formatMetadataText(parser);
    result.waveformText = renderWaveformASCII(result.originalSamples);

    if (auto it = result.otherChunks.find("LIST"); it != result.otherChunks.end())
        result.listTags = parseListChunk(it->second);

    const uint16_t numChannels = parser.getNumChannels();
    if (numChannels > 0 && !result.processedSamples.empty()) {
        const size_t totalFrames = result.processedSamples.size() / numChannels;
        const size_t framesToProcess = std::min<size_t>(totalFrames, 2048);

        if (framesToProcess > 1) {
            const std::vector<double> win = window::generate(window::Type::Hann, framesToProcess);

            for (uint16_t ch = 0; ch < numChannels; ++ch) {
                std::vector<float> block(framesToProcess);
                for (size_t i = 0; i < framesToProcess; ++i)
                    block[i] = result.processedSamples[i * numChannels + ch];

                window::apply(block, win);

                std::vector<double> blockD(block.begin(), block.end());
                auto spectrum = fft::transform(blockD);

                const size_t cutoff = spectrum.size() / 4;
                for (size_t k = cutoff; k < spectrum.size(); ++k)
                    spectrum[k] = {0.0, 0.0};

                auto reconstructed = fft::inverse(spectrum);
                for (size_t i = 0; i < framesToProcess; ++i)
                    result.processedSamples[i * numChannels + ch] =
                        std::clamp(static_cast<float>(reconstructed[i]), -1.0f, 1.0f);
            }

            result.fftReport = {true, framesToProcess, framesToProcess, framesToProcess / 4};
        }
    }

    return result;
}
