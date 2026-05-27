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
 * @brief Serialise a ListTags into a complete RIFF LIST/INFO chunk (header included).
 *        Empty tag fields are omitted. Returns an empty vector if all fields are empty.
 */
std::vector<char> buildListInfoChunk(const ListTags &tags);

/**
 * @brief Rewrite the LIST/INFO chunk in an existing WAV file in-place.
 *        Reads the file, replaces (or appends) the LIST chunk, and writes it back.
 * @throws std::runtime_error on I/O or format errors.
 */
void writeTagsToWav(const std::string &filePath, const ListTags &tags);

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
