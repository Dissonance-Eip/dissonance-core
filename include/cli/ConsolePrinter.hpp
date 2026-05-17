#pragma once

#include <array>
#include <string>
#include <vector>

#include "../audio/WavProcessor.hpp"

/** @brief ANSI colour codes used for coloured terminal output. */
extern const std::array<std::string, 5> COLORS;

/** @brief Print a single labelled field to stdout with colour formatting. */
void printField(const std::string &label, const std::string &value);

/**
 * @brief Renders WAV file information to the terminal.
 *
 * Loads the file on construction and exposes methods to print metadata,
 * unknown chunks, and an ASCII waveform. Check isValid() before printing.
 */
class ConsolePrinter {
  public:
    /** @param filename  Path to the WAV file to load and display. */
    explicit ConsolePrinter(const std::string &filename);

    /** @brief Returns false if the file could not be opened or parsed. */
    [[nodiscard]] bool isValid() const { return valid; }

    /** @brief Print WAV header fields (sample rate, channels, bit depth, etc.). */
    void printMetadata() const;

    /** @brief Print non-standard WAV chunks with a hex preview. */
    void printOtherChunks() const;

    /** @brief Print the ASCII waveform visualisation. */
    void printWaveform() const;

    /** @brief Print a LIST/INFO metadata chunk (title, artist, etc.). */
    void printListChunk(const std::vector<char> &value) const;

    /** @brief Print an unknown chunk as a hex dump. */
    static void printGenericChunk(const std::string &key, const std::vector<char> &value);

  private:
    ProcessedWav processed;
    bool valid = false;
    std::string filename;
};
