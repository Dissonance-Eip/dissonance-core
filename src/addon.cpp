#include <napi.h>
#include <string>

#include "AddonHelpers.hpp"
#include "WavProcessor.hpp"

namespace {

Napi::Value Process(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        throw Napi::TypeError::New(env, "Input path must be a string");
    }

    const std::string inputPath = info[0].As<Napi::String>();

    double gain = 0.8; // default reduce slightly
    if (info.Length() >= 2 && info[1].IsObject()) {
        Napi::Object opts = info[1].As<Napi::Object>();
        if (opts.Has("gain") && opts.Get("gain").IsNumber()) {
            gain = opts.Get("gain").As<Napi::Number>().DoubleValue();
        }
    }

    try {
        const ProcessedWav processed = processWavFile(inputPath, gain);

        Napi::Object listTags = Napi::Object::New(env);
        listTags.Set("title", Napi::String::New(env, processed.listTags.title));
        listTags.Set("artist", Napi::String::New(env, processed.listTags.artist));
        listTags.Set("comment", Napi::String::New(env, processed.listTags.comment));
        listTags.Set("date", Napi::String::New(env, processed.listTags.date));
        listTags.Set("software", Napi::String::New(env, processed.listTags.software));
        listTags.Set("genre", Napi::String::New(env, processed.listTags.genre));
        listTags.Set("copyright", Napi::String::New(env, processed.listTags.copyright));

        Napi::Object result = Napi::Object::New(env);
        result.Set("metadata", makeMetadataObject(env, processed.parser));
        result.Set("otherChunks", makeOtherChunks(env, processed.parser));
        result.Set("metadataText", Napi::String::New(env, processed.metadataText));
        result.Set("waveformText", Napi::String::New(env, processed.waveformText));
        result.Set("listTags", listTags);
        result.Set("processedPath", Napi::String::New(env, processed.processedPath));
        return result;
    } catch (const std::exception& e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("process", Napi::Function::New(env, Process));
    return exports;
}

} // namespace

NODE_API_MODULE(dissonance_core, Init)
