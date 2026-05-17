#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include "WavParser.hpp"

/** @brief Metadata fields extracted from a WAV LIST/INFO chunk. */
struct ListTags {
    std::string title;
    std::string artist;
    std::string comment;
    std::string date;
    std::string software;
    std::string genre;
    std::string copyright;
};

/**
 * @brief Parse a raw LIST chunk into human-readable tag fields.
 * @param value  Raw bytes of the LIST chunk (excluding the chunk header).
 */
ListTags parseListChunk(const std::vector<char> &value);

/**
 * @brief Format the WAV header fields as a multi-line human-readable string.
 * @param parser  A fully parsed Parser instance.
 */
std::string formatMetadataText(const Parser &parser);

/**
 * @brief Render an ASCII bar-chart waveform from normalised audio samples.
 * @param audioData  Normalised float samples in [-1, 1].
 * @param width      Waveform width in characters (columns).
 * @param height     Waveform height in characters (rows).
 */
std::string renderWaveformASCII(const std::vector<float> &audioData, int width = 200,
                                int height = 30);
