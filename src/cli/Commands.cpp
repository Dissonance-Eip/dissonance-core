#include "cli/Commands.hpp"
#include "cli/ConsolePrinter.hpp"
#include "audio/WavProcessor.hpp"
#include "audio/FFTProcessor.hpp"
#include "audio/WindowFunctions.hpp"
#include "core/Errors.hpp"
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <cstdlib>

void Commands::printUsage(const char *programName) {
    std::cout << "Usage: " << programName << " <command> <input_file> [options]\n\n"
              << "Commands:\n"
              << "  info          Print WAV file metadata and waveform\n"
              << "  process       Apply gain and spectral processing\n"
              << "  fft           Analyze FFT spectrum of a 512-frame block\n"
              << "\nOptions for 'fft':\n"
              << "  --offset <seconds>  Start offset into the file (default: 0)\n"
              << "  --bins <n>          Number of frequency bins to display (default: 16)\n"
              << "  --full              Average spectrum across entire file (Welch's method)\n"
              << "\nOptions for 'process':\n"
              << "  --gain <value>      Apply gain (default: 0.8)\n"
              << "  --output <path>     Output file path (default: <input>-processed.wav)\n"
              << "\nExamples:\n"
              << "  " << programName << " info sound.wav\n"
              << "  " << programName << " process sound.wav --gain 0.5 --output output.wav\n"
              << "  " << programName << " fft sound.wav\n";
}

int Commands::handleInfo(const std::string &inputPath) {
    auto printer = std::make_shared<ConsolePrinter>(inputPath);
    if (printer->isValid()) {
        std::cout << "=== WAV File Information ===\n";
        printer->printMetadata();
        std::cout << "\n=== Metadata Chunks ===\n";
        printer->printOtherChunks();
        std::cout << "\n=== Waveform ===\n";
        printer->printWaveform();
        return EXIT_SUCCESS;
    }
    throw dissonance::WavFormatError("WAV file invalid.");
}

int Commands::handleProcess(const std::string &inputPath, int argc, char **argv, int startIdx) {
    ProcessingOptions opts;

    for (int i = startIdx; i < argc; ++i) {
        if (std::string(argv[i]) == "--gain" && i + 1 < argc) {
            opts.gain = std::stod(argv[++i]);
        } else if (std::string(argv[i]) == "--output" && i + 1 < argc) {
            opts.outputPath = argv[++i];
        }
    }

    ProcessedWav result = processWavFile(inputPath, opts);

    std::cout << "\n=== Processing Complete ===\n";
    printField("Output", result.processedPath);
    printField("Gain applied", std::to_string(opts.gain));
    printField("FFT applied", result.fftReport.applied ? "yes" : "no");
    if (result.fftReport.applied) {
        printField("FFT frames", std::to_string(result.fftReport.framesProcessed));
        printField("FFT bins", std::to_string(result.fftReport.bins));
        printField("Cutoff bin", std::to_string(result.fftReport.cutoffBin));
    }
    return EXIT_SUCCESS;
}

int Commands::handleFft(const std::string &inputPath, int argc, char **argv, int startIdx) {
    double offsetSecs = 0.0;
    size_t topBins = 16;
    bool fullFile = false;

    for (int i = startIdx; i < argc; ++i) {
        if (std::string(argv[i]) == "--offset" && i + 1 < argc)
            offsetSecs = std::stod(argv[++i]);
        else if (std::string(argv[i]) == "--bins" && i + 1 < argc)
            topBins = static_cast<size_t>(std::stoul(argv[++i]));
        else if (std::string(argv[i]) == "--full")
            fullFile = true;
    }

    std::ifstream file(inputPath, std::ios::binary);
    if (!file.is_open())
        throw dissonance::WavFormatError("Failed to open file: " + inputPath);
    Parser parser = Parser::fromFile(file);
    file.close();

    const auto &samples = parser.getAudioData();
    const uint16_t numChannels = parser.getNumChannels();
    const uint32_t sampleRate = parser.getSampleRate();
    const size_t totalFrames = samples.size() / numChannels;

    constexpr size_t frameSize = 512;
    constexpr size_t hopSize = 256;

    const std::vector<double> win = window::generate(window::Type::Hann, frameSize);

    std::cout << "\n=== FFT Analysis ===\n";
    printField("Channels", std::to_string(numChannels));
    printField("Sample rate", std::to_string(sampleRate) + " Hz");

    std::vector<double> accumulated(frameSize, 0.0);
    size_t windowCount = 0;

    if (fullFile) {
        if (totalFrames < frameSize)
            throw dissonance::DspError("File too short for FFT analysis");

        printField("Mode", "full file (averaged periodogram)");
        printField("Window size", std::to_string(frameSize) + " frames");
        printField("Hop size", std::to_string(hopSize) + " frames");

        for (size_t offset = 0; offset + frameSize <= totalFrames; offset += hopSize) {
            std::vector<float> blockF(frameSize);
            for (size_t i = 0; i < frameSize; ++i)
                blockF[i] = samples[(offset + i) * numChannels];

            window::apply(blockF, win);

            std::vector<double> blockD(blockF.begin(), blockF.end());
            auto mags = fft::magnitude(fft::transform(blockD));

            for (size_t k = 0; k < frameSize; ++k)
                accumulated[k] += mags[k];
            ++windowCount;
        }

        printField("Windows averaged", std::to_string(windowCount));
    } else {
        const size_t offsetFrames =
            std::min<size_t>(static_cast<size_t>(offsetSecs * sampleRate), totalFrames);
        const size_t framesAvailable = totalFrames - offsetFrames;

        if (framesAvailable < 2)
            throw dissonance::DspError("Not enough samples for FFT analysis at this offset");

        const size_t framesToProcess = std::min<size_t>(framesAvailable, frameSize);
        printField("Offset",
                   std::to_string(offsetSecs) + " s (frame " + std::to_string(offsetFrames) + ")");
        printField("Frames analyzed", std::to_string(framesToProcess));

        std::vector<float> blockF(framesToProcess);
        for (size_t i = 0; i < framesToProcess; ++i)
            blockF[i] = samples[(offsetFrames + i) * numChannels];

        window::apply(blockF, win);

        std::vector<double> blockD(blockF.begin(), blockF.end());
        accumulated = fft::magnitude(fft::transform(blockD));
        accumulated.resize(frameSize, 0.0);
        windowCount = 1;
    }

    std::cout << "\nFrequency resolution: " << (static_cast<double>(sampleRate) / frameSize)
              << " Hz/bin\n\n";

    const size_t displayBins = std::min(topBins, frameSize / 2);
    std::cout << "Top " << displayBins << " frequency bins:\n";
    std::cout << "Bin\tFreq (Hz)\tMagnitude\n";
    std::cout << "---\t---------\t---------\n";

    for (size_t i = 0; i < displayBins; ++i) {
        double freq = (static_cast<double>(i) * sampleRate) / frameSize;
        double mag = accumulated[i] / static_cast<double>(windowCount);
        std::cout << i << "\t" << freq << "\t\t" << mag << "\n";
    }

    return EXIT_SUCCESS;
}
