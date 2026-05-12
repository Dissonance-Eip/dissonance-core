#include <napi.h>
#include <string>
#include <complex>

#include "addon/AddonHelpers.hpp"
#include "audio/WavProcessor.hpp"
#include "audio/WindowFunctions.hpp"
#include "audio/FFTProcessor.hpp"
#include "utils/WavUtils.hpp"
#include "core/Errors.hpp"

namespace {

std::vector<float> readFloat32Array(const Napi::CallbackInfo &info, uint32_t idx,
                                    const char *ctx) {
    Napi::Env env = info.Env();
    if (idx >= info.Length() || !info[idx].IsTypedArray())
        throw Napi::TypeError::New(env, std::string(ctx) + " must be a Float32Array");
    Napi::TypedArray ta = info[idx].As<Napi::TypedArray>();
    if (ta.TypedArrayType() != napi_float32_array)
        throw Napi::TypeError::New(env, std::string(ctx) + " must be a Float32Array");
    Napi::Float32Array fa = ta.As<Napi::Float32Array>();
    std::vector<float> result(fa.ElementLength());
    for (size_t i = 0; i < fa.ElementLength(); ++i)
        result[i] = fa[i];
    return result;
}

Napi::Float32Array makeFloat32Array(Napi::Env env, const std::vector<float> &values) {
    Napi::Float32Array arr = Napi::Float32Array::New(env, values.size());
    for (size_t i = 0; i < values.size(); ++i)
        arr[i] = values[i];
    return arr;
}

Napi::Float32Array makeFloat32ArrayFromDouble(Napi::Env env, const std::vector<double> &values) {
    Napi::Float32Array arr = Napi::Float32Array::New(env, values.size());
    for (size_t i = 0; i < values.size(); ++i)
        arr[i] = static_cast<float>(values[i]);
    return arr;
}

Napi::Object makeSpectrumObject(Napi::Env env,
                                const std::vector<std::complex<double>> &spectrum) {
    Napi::Float32Array real = Napi::Float32Array::New(env, spectrum.size());
    Napi::Float32Array imag = Napi::Float32Array::New(env, spectrum.size());
    for (size_t i = 0; i < spectrum.size(); ++i) {
        real[i] = static_cast<float>(spectrum[i].real());
        imag[i] = static_cast<float>(spectrum[i].imag());
    }
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("real", real);
    obj.Set("imag", imag);
    return obj;
}

std::vector<std::complex<double>> readSpectrumObject(Napi::Env env, const Napi::Object &obj,
                                                     const char *ctx) {
    if (!obj.Has("real") || !obj.Has("imag"))
        throw Napi::TypeError::New(env,
                                   std::string(ctx) + " must have real and imag Float32Arrays");
    Napi::Value realVal = obj.Get("real");
    Napi::Value imagVal = obj.Get("imag");
    if (!realVal.IsTypedArray() || !imagVal.IsTypedArray())
        throw Napi::TypeError::New(env, std::string(ctx) + " real and imag must be Float32Arrays");
    Napi::Float32Array realArr = realVal.As<Napi::Float32Array>();
    Napi::Float32Array imagArr = imagVal.As<Napi::Float32Array>();
    if (realArr.ElementLength() != imagArr.ElementLength())
        throw Napi::TypeError::New(env,
                                   std::string(ctx) + " real and imag must have the same length");
    std::vector<std::complex<double>> result(realArr.ElementLength());
    for (size_t i = 0; i < realArr.ElementLength(); ++i)
        result[i] = {static_cast<double>(realArr[i]), static_cast<double>(imagArr[i])};
    return result;
}

// --- Test/internal DSP exports ---

Napi::Value GenerateWindow(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsNumber())
        throw Napi::TypeError::New(env, "Expected (windowType: string, size: number)");

    const std::string typeStr = info[0].As<Napi::String>();
    const size_t size = info[1].As<Napi::Number>().Uint32Value();

    window::Type type;
    if (typeStr == "hann")
        type = window::Type::Hann;
    else if (typeStr == "hamming")
        type = window::Type::Hamming;
    else
        throw Napi::TypeError::New(env, "Window type must be 'hann' or 'hamming'");

    try {
        return makeFloat32ArrayFromDouble(env, window::generate(type, size));
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value ApplyWindow(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2)
        throw Napi::TypeError::New(env,
                                   "Expected (samples: Float32Array, window: Float32Array)");

    try {
        std::vector<float> samples = readFloat32Array(info, 0, "samples");
        std::vector<float> winF = readFloat32Array(info, 1, "window");

        if (samples.size() != winF.size())
            throw Napi::TypeError::New(env,
                                       "Sample and window arrays must have the same length");

        const std::vector<double> winD(winF.begin(), winF.end());
        window::apply(samples, winD);

        return makeFloat32Array(env, samples);
    } catch (const Napi::Error &) {
        throw;
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value FFT(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1)
        throw Napi::TypeError::New(env, "Expected (samples: Float32Array)");

    try {
        const std::vector<float> samples = readFloat32Array(info, 0, "samples");
        if (samples.empty())
            throw Napi::TypeError::New(env, "Input array cannot be empty");
        return makeSpectrumObject(env, fft::transform(samples));
    } catch (const Napi::Error &) {
        throw;
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value IFFT(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject())
        throw Napi::TypeError::New(
            env, "Expected (spectrum: {real: Float32Array, imag: Float32Array})");

    try {
        const auto spectrum = readSpectrumObject(env, info[0].As<Napi::Object>(), "spectrum");
        if (spectrum.empty())
            throw Napi::TypeError::New(env, "Spectrum cannot be empty");
        return makeFloat32ArrayFromDouble(env, fft::inverse(spectrum));
    } catch (const Napi::Error &) {
        throw;
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value FFTMagnitude(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject())
        throw Napi::TypeError::New(
            env, "Expected (spectrum: {real: Float32Array, imag: Float32Array})");

    try {
        const auto spectrum = readSpectrumObject(env, info[0].As<Napi::Object>(), "spectrum");
        return makeFloat32ArrayFromDouble(env, fft::magnitude(spectrum));
    } catch (const Napi::Error &) {
        throw;
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value Process(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString())
        throw Napi::TypeError::New(env, "Input path must be a string");

    const std::string inputPath = info[0].As<Napi::String>();

    ProcessingOptions opts;
    if (info.Length() >= 2 && info[1].IsObject()) {
        Napi::Object o = info[1].As<Napi::Object>();
        if (o.Has("gain") && o.Get("gain").IsNumber())
            opts.gain = o.Get("gain").As<Napi::Number>().DoubleValue();
        if (o.Has("outputPath") && o.Get("outputPath").IsString())
            opts.outputPath = o.Get("outputPath").As<Napi::String>().Utf8Value();
    }

    try {
        const ProcessedWav processed = processWavFile(inputPath, opts);

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
        result.Set("fftApplied", Napi::Boolean::New(env, processed.fftReport.applied));
        result.Set("fftFramesProcessed",
                   Napi::Number::New(env, processed.fftReport.framesProcessed));
        result.Set("fftBins", Napi::Number::New(env, processed.fftReport.bins));
        result.Set("fftCutoffBin", Napi::Number::New(env, processed.fftReport.cutoffBin));
        return result;
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value Inspect(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString())
        throw Napi::TypeError::New(env, "Input path must be a string");

    const std::string inputPath = info[0].As<Napi::String>();

    try {
        std::ifstream file(inputPath, std::ios::binary);
        if (!file.is_open())
            throw dissonance::WavFormatError("Failed to open file: " + inputPath);

        const Parser parser = Parser::fromFile(file, false);

        Napi::Object listTags = Napi::Object::New(env);
        if (auto it = parser.getOtherChunks().find("LIST"); it != parser.getOtherChunks().end()) {
            const ListTags tags = parseListChunk(it->second);
            listTags.Set("title", Napi::String::New(env, tags.title));
            listTags.Set("artist", Napi::String::New(env, tags.artist));
            listTags.Set("comment", Napi::String::New(env, tags.comment));
            listTags.Set("date", Napi::String::New(env, tags.date));
            listTags.Set("software", Napi::String::New(env, tags.software));
            listTags.Set("genre", Napi::String::New(env, tags.genre));
            listTags.Set("copyright", Napi::String::New(env, tags.copyright));
        } else {
            for (const char *k :
                 {"title", "artist", "comment", "date", "software", "genre", "copyright"})
                listTags.Set(k, Napi::String::New(env, ""));
        }

        Napi::Object result = Napi::Object::New(env);
        result.Set("metadata", makeMetadataObject(env, parser));
        result.Set("otherChunks", makeOtherChunks(env, parser));
        result.Set("metadataText", Napi::String::New(env, formatMetadataText(parser)));
        result.Set("listTags", listTags);
        return result;
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("process", Napi::Function::New(env, Process));
    exports.Set("inspect", Napi::Function::New(env, Inspect));
    exports.Set("generateWindow", Napi::Function::New(env, GenerateWindow));
    exports.Set("applyWindow", Napi::Function::New(env, ApplyWindow));
    exports.Set("fft", Napi::Function::New(env, FFT));
    exports.Set("ifft", Napi::Function::New(env, IFFT));
    exports.Set("fftMagnitude", Napi::Function::New(env, FFTMagnitude));
    return exports;
}

} // namespace

NODE_API_MODULE(dissonance_core, Init)
