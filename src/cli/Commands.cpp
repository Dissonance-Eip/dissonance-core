/**
 * @file Commands.cpp
 * @brief CLI sub-command implementations: info, process, fft.
 */

#include "cli/Commands.hpp"
#include "cli/ConsolePrinter.hpp"
#include "audio/WavProcessor.hpp"
#include "audio/FFTProcessor.hpp"
#include "audio/WindowFunctions.hpp"
#include "core/Errors.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>

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
              << "  --sort              Sort displayed bins by magnitude (loudest first)\n"
              << "\nOptions for 'process':\n"
              << "  --gain <value>          Apply gain (default: 0.8)\n"
              << "  --perturbation <0..1>   Per-mode noise strength (default: 0.5)\n"
              << "  --mode <name>           Perturbation mode (";
    bool first = true;
    const std::vector<std::string> knownModes = {"white_noise", "phase_distortion", "spectral_gate",
                                                 "pink_noise"};
    for (const auto &m : knownModes) {
        if (!first)
            std::cout << ", ";
        first = false;
        std::cout << m;
    }
    std::cout << "). Repeatable — modes stack in order.\n"
              << "  --output <path>         Output file path (default: <input>-processed.wav)\n"
              << "\nExamples:\n"
              << "  " << programName << " info sound.wav\n"
              << "  " << programName
              << " process sound.wav --gain 0.5 --perturbation 0.8 --output output.wav\n"
              << "  " << programName << " fft sound.wav --full --sort\n";
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
        } else if (std::string(argv[i]) == "--perturbation" && i + 1 < argc) {
            opts.perturbation = static_cast<float>(std::stod(argv[++i]));
        } else if (std::string(argv[i]) == "--mode" && i + 1 < argc) {
            opts.perturbationModes.push_back(argv[++i]);
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
    if (!opts.perturbationModes.empty()) {
        std::string modesStr;
        for (size_t i = 0; i < opts.perturbationModes.size(); ++i) {
            if (i > 0)
                modesStr += ", ";
            modesStr += opts.perturbationModes[i];
        }
        printField("Perturbation modes", modesStr);
        printField("Perturbation strength", std::to_string(opts.perturbation));
        printField("Perturbation RMS", std::to_string(result.perturbationRmsDbfs) + " dBFS");
    } else if (opts.perturbation > 0.0f) {
        printField("Perturbation strength", std::to_string(opts.perturbation));
        printField("Perturbation RMS", std::to_string(result.perturbationRmsDbfs) + " dBFS");
    }
    return EXIT_SUCCESS;
}

int Commands::handleFft(const std::string &inputPath, int argc, char **argv, int startIdx) {
    double offsetSecs = 0.0;
    size_t topBins = 16;
    bool fullFile = false;
    bool sortByMag = false;

    for (int i = startIdx; i < argc; ++i) {
        if (std::string(argv[i]) == "--offset" && i + 1 < argc)
            offsetSecs = std::stod(argv[++i]);
        else if (std::string(argv[i]) == "--bins" && i + 1 < argc)
            topBins = static_cast<size_t>(std::stoul(argv[++i]));
        else if (std::string(argv[i]) == "--full")
            fullFile = true;
        else if (std::string(argv[i]) == "--sort")
            sortByMag = true;
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

    const std::vector<float> win = window::generate(window::Type::Hann, frameSize);

    std::cout << "\n=== FFT Analysis ===\n";
    printField("Channels", std::to_string(numChannels));
    printField("Sample rate", std::to_string(sampleRate) + " Hz");

    std::vector<float> accumulated(frameSize, 0.0);
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

            auto mags = fft::magnitude(fft::transform(blockF));
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

        accumulated = fft::magnitude(fft::transform(blockF));
        accumulated.resize(frameSize, 0.0);
        windowCount = 1;
    }

    // Normalize to per-window average — only use positive-frequency half
    const size_t halfBins = frameSize / 2;
    std::vector<float> mags(halfBins);
    for (size_t k = 0; k < halfBins; ++k)
        mags[k] = accumulated[k] / static_cast<double>(windowCount);

    // --- Summary stats ---
    const size_t peakBin =
        static_cast<size_t>(std::max_element(mags.begin() + 1, mags.end()) - mags.begin());
    const double peakFreq = (static_cast<double>(peakBin) * sampleRate) / frameSize;
    const double freqRes = static_cast<double>(sampleRate) / frameSize;

    double sumMag = 0.0, sumWeighted = 0.0;
    for (size_t k = 1; k < halfBins; ++k) {
        double freq = static_cast<double>(k) * freqRes;
        sumMag += mags[k];
        sumWeighted += freq * mags[k];
    }
    const double centroid = (sumMag > 0.0) ? (sumWeighted / sumMag) : 0.0;
    const double dcRatio = (mags[peakBin] > 0.0) ? (mags[0] / mags[peakBin]) : 0.0;

    std::cout << "\n=== Spectrum Summary ===\n";
    std::cout << std::fixed << std::setprecision(1);
    printField("Peak frequency", std::to_string(static_cast<int>(std::round(peakFreq))) + " Hz" +
                                     " (bin " + std::to_string(peakBin) + ")");
    printField("Spectral centroid", std::to_string(static_cast<int>(std::round(centroid))) + " Hz");
    if (dcRatio > 0.5)
        printField("DC bias", "HIGH (" + std::to_string(static_cast<int>(dcRatio * 100)) +
                                  "% of peak) — consider high-pass filtering");

    // --- Build display list ---
    std::vector<size_t> indices(std::min(topBins, halfBins));
    if (sortByMag) {
        std::vector<size_t> all(halfBins);
        std::iota(all.begin(), all.end(), 0);
        std::partial_sort(all.begin(), all.begin() + static_cast<ptrdiff_t>(indices.size()),
                          all.end(), [&](size_t a, size_t b) { return mags[a] > mags[b]; });
        std::copy(all.begin(), all.begin() + static_cast<ptrdiff_t>(indices.size()),
                  indices.begin());
    } else {
        std::iota(indices.begin(), indices.end(), 0);
    }

    const double maxMag = *std::max_element(mags.begin(), mags.end());
    constexpr int barWidth = 30;

    std::cout << "\n"
              << (sortByMag ? "Top" : "First") << " " << indices.size() << " frequency bins"
              << (sortByMag ? " (sorted by magnitude)" : "") << ":\n";
    std::cout << std::left << std::setw(5) << "Bin" << std::setw(12) << "Freq (Hz)" << std::setw(10)
              << "Magnitude" << "Bar\n";
    std::cout << std::string(5, '-') << std::string(12, '-') << std::string(10, '-')
              << std::string(barWidth, '-') << "\n";

    std::cout << std::fixed << std::setprecision(2);
    for (size_t idx : indices) {
        double freq = static_cast<double>(idx) * freqRes;
        double mag = mags[idx];
        int bars = (maxMag > 0.0) ? static_cast<int>(mag / maxMag * barWidth) : 0;

        std::cout << std::left << std::setw(5) << idx << std::setw(12) << freq << std::setw(10)
                  << mag << std::string(bars, '#') << "\n";
    }

    return EXIT_SUCCESS;
}
