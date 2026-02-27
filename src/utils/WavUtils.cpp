#include "utils/WavUtils.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

ListTags parseListChunk(const std::vector<char> &value) {
    ListTags tags{};

    const std::unordered_map<std::string, std::string *> chunkMap = {
        {"INAM", &tags.title},     {"IART", &tags.artist},   {"ICMT", &tags.comment},
        {"ICRD", &tags.date},      {"ISFT", &tags.software}, {"IGNR", &tags.genre},
        {"ICOP", &tags.copyright},
    };

    // Skip the 4-byte LIST type field (e.g., "INFO")
    size_t i = 4;
    while (i + 8 <= value.size()) { // need at least id + size
        const std::string chunkId(value.begin() + i, value.begin() + i + 4);
        i += 4;
        const uint32_t chunkSize = *reinterpret_cast<const uint32_t *>(&value[i]);
        i += 4;

        const size_t end = std::min(value.size(), i + chunkSize);
        if (chunkMap.count(chunkId)) {
            auto *field = chunkMap.at(chunkId);
            *field = std::string(value.begin() + i, value.begin() + end);
            field->erase(
                std::find_if(field->rbegin(), field->rend(),
                             [](unsigned char ch) { return !std::isspace(ch) && ch != '\0'; })
                    .base(),
                field->end());
        }
        i += chunkSize;
        // WAV chunks must be word-aligned; skip padding byte if chunk size is odd
        if (chunkSize % 2 == 1) {
            i += 1;
        }
    }

    if (tags.date.size() == 8) {
        tags.date =
            tags.date.substr(0, 4) + "-" + tags.date.substr(4, 2) + "-" + tags.date.substr(6, 2);
    }

    return tags;
}

std::string formatMetadataText(const Parser &parser) {
    std::ostringstream oss;
    oss << "Chunk size: " << parser.getChunkSize() << '\n';
    oss << "Audio format: " << parser.getAudioFormat() << '\n';
    oss << "Number of channels: " << parser.getNumChannels() << '\n';
    oss << "Sample rate: " << parser.getSampleRate() << '\n';
    oss << "Byte rate: " << parser.getByteRate() << '\n';
    oss << "Block align: " << parser.getBlockAlign() << '\n';
    oss << "Bits per sample: " << parser.getBitsPerSample() << '\n';
    oss << "Data size: " << parser.getSubchunk2Size() << '\n';
    return oss.str();
}

std::string renderWaveformASCII(const std::vector<int16_t> &audioData, int width, int height) {
    if (audioData.empty()) {
        return "(no audio data)";
    }

    const int16_t maxSample = *std::max_element(audioData.begin(), audioData.end());
    const int16_t minSample = *std::min_element(audioData.begin(), audioData.end());
    const int range = std::max<int>(1, maxSample - minSample);

    std::vector<std::string> rows(static_cast<size_t>(height),
                                  std::string(static_cast<size_t>(width), ' '));
    const int mid = height / 2;
    int prevY = mid;

    const std::size_t lastIdx = audioData.size() > 0 ? audioData.size() - 1 : 0;
    for (int i = 0; i < width; ++i) {
        const std::size_t index =
            static_cast<std::size_t>((static_cast<double>(i) / std::max(1, width - 1)) * lastIdx);
        const int sample = audioData[index];

        int y = mid + (sample - minSample) * (height - 1) / range - mid;
        y = std::clamp(y, 0, height - 1);

        if (y != prevY) {
            const int start = std::min(y, prevY);
            const int end = std::max(y, prevY);
            for (int j = start; j <= end; ++j) {
                rows[static_cast<size_t>(j)][static_cast<size_t>(i)] = '|';
            }
        } else {
            rows[static_cast<size_t>(y)][static_cast<size_t>(i)] = '|';
        }
        prevY = y;
    }

    std::ostringstream oss;
    for (const auto &line : rows) {
        oss << line << '\n';
    }
    return oss.str();
}
