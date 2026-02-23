#include "cli/Commands.hpp"
#include "cli/WavGUI.hpp"
#include "audio/WavProcessor.hpp"
#include "audio/FFTProcessor.hpp"
#include "audio/WindowFunctions.hpp"
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
              << "  fft           Analyze FFT spectrum of first block\n"
              << "\nOptions for 'process':\n"
              << "  --gain <value>      Apply gain (default: 0.8)\n"
              << "  --output <path>     Output file path (default: <input>-processed.wav)\n"
              << "\nExamples:\n"
              << "  " << programName << " info sound.wav\n"
              << "  " << programName << " process sound.wav --gain 0.5 --output output.wav\n"
              << "  " << programName << " fft sound.wav\n";
}

int Commands::handleInfo(const std::string &inputPath) {
    auto gui = std::make_shared<GUI>(inputPath);
    if (gui->isValid()) {
        std::cout << "=== WAV File Information ===\n";
        gui->printMetadata();
        std::cout << "\n=== Metadata Chunks ===\n";
        gui->printOtherChunks();
        std::cout << "\n=== Waveform ===\n";
        gui->printWaveform();
        return EXIT_SUCCESS;
    }
    throw std::runtime_error("WAV file invalid.");
}

int Commands::handleProcess(const std::string &inputPath, int argc, char **argv, int startIdx) {
    double gain = 0.8;
    std::string outputPath = "";

    for (int i = startIdx; i < argc; ++i) {
        if (std::string(argv[i]) == "--gain" && i + 1 < argc) {
            gain = std::stod(argv[++i]);
        } else if (std::string(argv[i]) == "--output" && i + 1 < argc) {
            outputPath = argv[++i];
        }
    }

    ProcessedWav result = processWavFile(inputPath, gain, outputPath);

    std::cout << "\n=== Processing Complete ===\n";
    printField("Output", result.processedPath);
    printField("Gain applied", std::to_string(gain));
    printField("FFT applied", result.fftApplied ? "yes" : "no");
    if (result.fftApplied) {
        printField("FFT frames", std::to_string(result.fftFramesProcessed));
        printField("FFT bins", std::to_string(result.fftBins));
        printField("Cutoff bin", std::to_string(result.fftCutoffBin));
    }
    return EXIT_SUCCESS;
}

int Commands::handleFft(const std::string &inputPath) {
    Parser parser;
    std::ifstream file(inputPath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + inputPath);
    }
    parser.readFromFile(file);
    file.close();

    const auto &samples = parser.getAudioData();
    const uint16_t numChannels = parser.getNumChannels();
    const size_t totalFrames = samples.size() / numChannels;
    const size_t framesToProcess = std::min<size_t>(totalFrames, 512);

    if (framesToProcess < 2) {
        throw std::runtime_error("Not enough samples for FFT analysis");
    }

    const std::vector<double> window =
        WindowFunctions::generate(WindowFunctions::Type::Hann, framesToProcess);

    std::cout << "\n=== FFT Analysis ===\n";
    printField("Channels", std::to_string(numChannels));
    printField("Sample rate", std::to_string(parser.getSampleRate()) + " Hz");
    printField("Frames analyzed", std::to_string(framesToProcess));
    std::cout << "\nFrequency resolution: "
              << (static_cast<double>(parser.getSampleRate()) / framesToProcess) << " Hz/bin\n\n";

    std::vector<double> block(framesToProcess);
    for (size_t i = 0; i < framesToProcess; ++i) {
        block[i] = static_cast<double>(samples[i * numChannels]);
    }

    WindowFunctions::apply(block, window);
    auto spectrum = FFTProcessor::fft(block);
    auto magnitudes = FFTProcessor::magnitude(spectrum);

    std::cout << "Top 16 frequency bins:\n";
    std::cout << "Bin\tFreq (Hz)\tMagnitude\n";
    std::cout << "---\t---------\t---------\n";

    for (size_t i = 0; i < std::min<size_t>(16, magnitudes.size() / 2); ++i) {
        double freq = (static_cast<double>(i) * parser.getSampleRate()) / framesToProcess;
        std::cout << i << "\t" << freq << "\t\t" << magnitudes[i] << "\n";
    }

    return EXIT_SUCCESS;
}
