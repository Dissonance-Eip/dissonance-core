#include "addon/AddonHelpers.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

std::string toHexPreview(const std::vector<char> &data, std::size_t maxBytes) {
    std::ostringstream oss;
    const std::size_t limit = std::min(maxBytes, data.size());
    for (std::size_t i = 0; i < limit; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << (static_cast<int>(static_cast<unsigned char>(data[i])));
        if (i + 1 < limit) {
            oss << " ";
        }
    }
    if (data.size() > limit) {
        oss << " ...";
    }
    return oss.str();
}

Napi::Object makeMetadataObject(Napi::Env env, const Parser &parser) {
    Napi::Object meta = Napi::Object::New(env);
    meta.Set("riff", Napi::String::New(env, parser.getRiff()));
    meta.Set("chunkSize", Napi::Number::New(env, parser.getChunkSize()));
    meta.Set("wave", Napi::String::New(env, parser.getWave()));
    meta.Set("fmt", Napi::String::New(env, parser.getFmt()));
    meta.Set("subchunk1Size", Napi::Number::New(env, parser.getSubchunk1Size()));
    meta.Set("audioFormat", Napi::Number::New(env, parser.getAudioFormat()));
    meta.Set("numChannels", Napi::Number::New(env, parser.getNumChannels()));
    meta.Set("sampleRate", Napi::Number::New(env, parser.getSampleRate()));
    meta.Set("byteRate", Napi::Number::New(env, parser.getByteRate()));
    meta.Set("blockAlign", Napi::Number::New(env, parser.getBlockAlign()));
    meta.Set("bitsPerSample", Napi::Number::New(env, parser.getBitsPerSample()));
    meta.Set("data", Napi::String::New(env, parser.getData()));
    meta.Set("subchunk2Size", Napi::Number::New(env, parser.getSubchunk2Size()));

    // `audioData` may be intentionally skipped for fast header inspection.
    // Derive sample count from header sizes when possible.
    const uint16_t bps = parser.getBitsPerSample();
    const uint32_t dataBytes = parser.getSubchunk2Size();
    uint32_t numSamples = 0;
    if (bps > 0 && bps % 8 == 0) {
        const uint32_t bytesPerSample = bps / 8;
        if (bytesPerSample > 0) {
            numSamples = dataBytes / bytesPerSample;
        }
    }
    if (numSamples == 0) {
        numSamples = static_cast<uint32_t>(parser.getAudioData().size());
    }
    meta.Set("numSamples", Napi::Number::New(env, numSamples));
    return meta;
}

Napi::Array makeOtherChunks(Napi::Env env, const Parser &parser) {
    const auto &chunks = parser.getOtherChunks();
    Napi::Array arr = Napi::Array::New(env, chunks.size());
    std::size_t idx = 0;
    for (const auto &[id, blob] : chunks) {
        Napi::Object entry = Napi::Object::New(env);
        entry.Set("id", Napi::String::New(env, id));
        entry.Set("size", Napi::Number::New(env, blob.size()));
        entry.Set("preview", Napi::String::New(env, toHexPreview(blob)));
        arr.Set(idx++, entry);
    }
    return arr;
}
