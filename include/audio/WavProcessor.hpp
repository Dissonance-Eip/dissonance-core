#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../utils/WavParser.hpp"
#include "../utils/WavUtils.hpp"

/** @brief Statistics collected from the WindowedFFTStage after processing. */
struct FFTReport {
    bool applied = false;       ///< True if the FFT stage ran on at least one frame.
    size_t framesProcessed = 0; ///< Number of FFT hops processed.
    size_t bins = 0;            ///< FFT frame size (= number of frequency bins).
    size_t cutoffBin = 0;       ///< First bin that was zeroed (low-pass cutoff index).
};

/** @brief Tunable parameters for processWavFile(). */
struct ProcessingOptions {
    double gain = 0.8;                       ///< Linear amplitude gain applied after FFT filtering.
    std::string outputPath;                  ///< Output file path. Auto-generated if empty.
    std::function<void(float)> progressCallback; ///< Optional progress callback in [0, 1].
};

/** @brief Everything produced by processWavFile() — parsed metadata plus the processed audio. */
struct ProcessedWav {
    Parser parser;                                             ///< Parsed WAV header and metadata.
    std::unordered_map<std::string, std::vector<char>> otherChunks; ///< Non-fmt/data WAV chunks.
    std::vector<float> originalSamples;  ///< Raw samples before processing.
    std::vector<float> processedSamples; ///< Samples after the full pipeline.
    ListTags listTags;                   ///< Parsed LIST/INFO tag fields.
    std::string metadataText;            ///< Human-readable metadata summary.
    std::string waveformText;            ///< ASCII waveform rendering.
    std::string processedPath;           ///< Absolute path of the written output file.
    FFTReport fftReport;                 ///< FFT stage statistics.
};

/**
 * @brief Run the full audio protection pipeline on a WAV file.
 *
 * Reads the file at inputPath, applies WindowedFFTStage (spectral low-pass
 * filter) followed by GainStage, writes the result to opts.outputPath (or
 * an auto-generated path next to the input), and returns a ProcessedWav
 * containing metadata and processing statistics.
 *
 * @throws dissonance::WavFormatError  If the file cannot be opened or parsed.
 */
ProcessedWav processWavFile(const std::string &inputPath, const ProcessingOptions &opts = {});
