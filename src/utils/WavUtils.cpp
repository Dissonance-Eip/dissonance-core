/**
 * @file WavUtils.cpp
 * @brief WAV utility functions: LIST chunk parsing, metadata formatting, ASCII waveform.
 */

#include "utils/WavUtils.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
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

namespace {

// Write a little-endian uint32 into a byte vector.
void pushU32LE(std::vector<char> &buf, uint32_t v) {
    buf.push_back(static_cast<char>(v & 0xFF));
    buf.push_back(static_cast<char>((v >> 8) & 0xFF));
    buf.push_back(static_cast<char>((v >> 16) & 0xFF));
    buf.push_back(static_cast<char>((v >> 24) & 0xFF));
}

} // namespace

std::vector<char> buildListInfoChunk(const ListTags &tags) {
    struct TagDef {
        const char *id;
        const std::string &value;
    };
    const TagDef defs[] = {
        {"INAM", tags.title},     {"IART", tags.artist}, {"ICMT", tags.comment},
        {"ICRD", tags.date},      {"IGNR", tags.genre},  {"ISFT", tags.software},
        {"ICOP", tags.copyright},
    };

    // Build INFO payload: "INFO" + sub-chunks for each non-empty field.
    std::vector<char> info;
    info.insert(info.end(), {'I', 'N', 'F', 'O'});

    for (const auto &def : defs) {
        if (def.value.empty())
            continue;
        const uint32_t strSize = static_cast<uint32_t>(def.value.size()) + 1; // include null
        info.insert(info.end(), def.id, def.id + 4);
        pushU32LE(info, strSize);
        info.insert(info.end(), def.value.begin(), def.value.end());
        info.push_back('\0');
        if (strSize % 2 != 0)
            info.push_back('\0'); // word-align
    }

    // If nothing was written (only "INFO" header present), return empty.
    if (info.size() == 4)
        return {};

    // Wrap in LIST chunk header.
    std::vector<char> chunk;
    chunk.insert(chunk.end(), {'L', 'I', 'S', 'T'});
    pushU32LE(chunk, static_cast<uint32_t>(info.size()));
    chunk.insert(chunk.end(), info.begin(), info.end());
    return chunk;
}

void writeTagsToWav(const std::string &filePath, const ListTags &tags) {
    // Read entire file.
    std::ifstream in(filePath, std::ios::binary);
    if (!in.is_open())
        throw std::runtime_error("Cannot open file for tag write: " + filePath);
    std::vector<char> file((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    if (file.size() < 12 || std::string(file.data(), 4) != "RIFF" ||
        std::string(file.data() + 8, 4) != "WAVE")
        throw std::runtime_error("Not a valid RIFF/WAVE file: " + filePath);

    // Walk chunks. Keep everything except existing LIST chunks.
    // The parser breaks after reading the data chunk, so LIST must sit BEFORE data
    // to be visible on the next readMetadata call.
    std::vector<char> beforeData; // fmt + other non-LIST, non-data chunks
    std::vector<char> dataChunk;  // the data chunk verbatim
    size_t offset = 12;
    while (offset + 8 <= file.size()) {
        const std::string id(file.data() + offset, 4);
        uint32_t chunkSize = 0;
        std::memcpy(&chunkSize, file.data() + offset + 4, 4);
        const size_t total = 8 + chunkSize + (chunkSize % 2);
        const size_t end = std::min(offset + total, file.size());

        if (id == "LIST") {
            // drop existing LIST chunk — we'll rebuild it
        } else if (id == "data") {
            dataChunk.insert(dataChunk.end(), file.begin() + static_cast<std::ptrdiff_t>(offset),
                             file.begin() + static_cast<std::ptrdiff_t>(end));
        } else {
            beforeData.insert(beforeData.end(), file.begin() + static_cast<std::ptrdiff_t>(offset),
                              file.begin() + static_cast<std::ptrdiff_t>(end));
        }
        offset += total;
    }

    // Build new LIST/INFO chunk and place it before the data chunk.
    const auto listChunk = buildListInfoChunk(tags);
    std::vector<char> chunks;
    chunks.insert(chunks.end(), beforeData.begin(), beforeData.end());
    chunks.insert(chunks.end(), listChunk.begin(), listChunk.end());
    chunks.insert(chunks.end(), dataChunk.begin(), dataChunk.end());

    // Rebuild file with corrected RIFF size.
    const uint32_t riffSize = 4 + static_cast<uint32_t>(chunks.size()); // "WAVE" + chunks
    std::vector<char> out;
    out.reserve(12 + chunks.size());
    out.insert(out.end(), {'R', 'I', 'F', 'F'});
    pushU32LE(out, riffSize);
    out.insert(out.end(), {'W', 'A', 'V', 'E'});
    out.insert(out.end(), chunks.begin(), chunks.end());

    std::ofstream outf(filePath, std::ios::binary | std::ios::trunc);
    if (!outf.is_open())
        throw std::runtime_error("Cannot write file: " + filePath);
    outf.write(out.data(), static_cast<std::streamsize>(out.size()));
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

std::string renderWaveformASCII(const std::vector<float> &audioData, int width, int height) {
    if (audioData.empty()) {
        return "(no audio data)";
    }

    const float maxSample = *std::max_element(audioData.begin(), audioData.end());
    const float minSample = *std::min_element(audioData.begin(), audioData.end());
    const float range = std::max(1e-6f, maxSample - minSample);

    std::vector<std::string> rows(static_cast<size_t>(height),
                                  std::string(static_cast<size_t>(width), ' '));
    const int mid = height / 2;
    int prevY = mid;

    const std::size_t lastIdx = audioData.size() > 0 ? audioData.size() - 1 : 0;
    for (int i = 0; i < width; ++i) {
        const std::size_t index =
            static_cast<std::size_t>((static_cast<double>(i) / std::max(1, width - 1)) * lastIdx);
        const float sample = audioData[index];

        int y = static_cast<int>((sample - minSample) / range * (height - 1));
        y = std::clamp(y, 0, height - 1);

        const int lo = std::min(y, prevY);
        const int hi = std::max(y, prevY);
        for (int j = lo; j <= hi; ++j)
            rows[static_cast<size_t>(j)][static_cast<size_t>(i)] = '|';
        prevY = y;
    }

    std::ostringstream oss;
    for (const auto &line : rows) {
        oss << line << '\n';
    }
    return oss.str();
}
