#pragma once

#include <string>

/**
 * @brief CLI command handlers for the dissonance.core executable.
 *
 * Each static method corresponds to one CLI sub-command and returns an
 * exit code suitable for passing to std::exit / returning from main().
 */
class Commands {
  public:
    /** @brief Print usage/help to stderr. */
    static void printUsage(const char *programName);

    /** @brief Handle the `info` command — print WAV metadata and waveform. */
    static int handleInfo(const std::string &inputPath);

    /**
     * @brief Handle the `process` command — apply gain and spectral filtering.
     * @param startIdx  argv index of the first option after the input path.
     */
    static int handleProcess(const std::string &inputPath, int argc, char **argv, int startIdx);

    /**
     * @brief Handle the `fft` command — display the frequency spectrum.
     * @param startIdx  argv index of the first option after the input path.
     */
    static int handleFft(const std::string &inputPath, int argc, char **argv, int startIdx);

    /**
     * @brief Handle the `bark` command — display Bark band mapping for WAV bins.
     * Shows which psychoacoustic Bark band each FFT bin falls into.
     *
     * Syntax: dissonance.core bark <input.wav> [--frame-size 2048]
     */
    static int handleBark(const std::string &inputPath, int argc, char **argv, int startIdx);
};
