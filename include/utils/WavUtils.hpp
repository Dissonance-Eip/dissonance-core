#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include "WavParser.hpp"

struct ListTags {
    std::string title;
    std::string artist;
    std::string comment;
    std::string date;
    std::string software;
    std::string genre;
    std::string copyright;
};

ListTags parseListChunk(const std::vector<char> &value);
std::string formatMetadataText(const Parser &parser);
std::string renderWaveformASCII(const std::vector<float> &audioData, int width = 200,
                                int height = 30);
