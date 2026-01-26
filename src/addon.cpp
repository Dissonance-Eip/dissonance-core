#include <napi.h>
#include <string>

#include "AddonHelpers.hpp"
#include "WavProcessor.hpp"
#include "WindowFunctions.hpp"

namespace {

Napi::Value GenerateWindow(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsNumber()) {
        throw Napi::TypeError::New(env, "Expected (windowType: string, size: number)");
    }

    const std::string typeStr = info[0].As<Napi::String>();
    const size_t size = info[1].As<Napi::Number>().Uint32Value();

    WindowFunctions::Type type;
    if (typeStr == "hann") {
        type = WindowFunctions::Type::Hann;
    } else if (typeStr == "hamming") {
        type = WindowFunctions::Type::Hamming;
    } else {
        throw Napi::TypeError::New(env, "Window type must be 'hann' or 'hamming'");
    }

    try {
        const std::vector<double> window = WindowFunctions::generate(type, size);
        
        Napi::Array result = Napi::Array::New(env, window.size());
        for (size_t i = 0; i < window.size(); ++i) {
            result[i] = Napi::Number::New(env, window[i]);
        }
        
        return result;
    } catch (const std::exception& e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value ApplyWindow(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsArray() || !info[1].IsArray()) {
        throw Napi::TypeError::New(env, "Expected (samples: number[], window: number[])");
    }

    Napi::Array samplesArray = info[0].As<Napi::Array>();
    Napi::Array windowArray = info[1].As<Napi::Array>();

    if (samplesArray.Length() != windowArray.Length()) {
        throw Napi::TypeError::New(env, "Sample and window arrays must have the same length");
    }

    try {
        std::vector<double> samples(samplesArray.Length());
        std::vector<double> window(windowArray.Length());

        for (uint32_t i = 0; i < samplesArray.Length(); ++i) {
            samples[i] = samplesArray.Get(i).As<Napi::Number>().DoubleValue();
            window[i] = windowArray.Get(i).As<Napi::Number>().DoubleValue();
        }

        WindowFunctions::apply(samples, window);

        Napi::Array result = Napi::Array::New(env, samples.size());
        for (size_t i = 0; i < samples.size(); ++i) {
            result[i] = Napi::Number::New(env, samples[i]);
        }

        return result;
    } catch (const std::exception& e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value Process(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        throw Napi::TypeError::New(env, "Input path must be a string");
    }

    const std::string inputPath = info[0].As<Napi::String>();

    double gain = 0.8; // default reduce slightly
    std::string outputPath = "";
    if (info.Length() >= 2 && info[1].IsObject()) {
        Napi::Object opts = info[1].As<Napi::Object>();
        if (opts.Has("gain") && opts.Get("gain").IsNumber()) {
            gain = opts.Get("gain").As<Napi::Number>().DoubleValue();
        }
        if (opts.Has("outputPath") && opts.Get("outputPath").IsString()) {
            outputPath = opts.Get("outputPath").As<Napi::String>();
        }
    }

    try {
        const ProcessedWav processed = processWavFile(inputPath, gain, outputPath);

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
    exports.Set("generateWindow", Napi::Function::New(env, GenerateWindow));
    exports.Set("applyWindow", Napi::Function::New(env, ApplyWindow));
    return exports;
}

} // namespace

NODE_API_MODULE(dissonance_core, Init)
