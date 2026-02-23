#include <napi.h>
#include <string>
#include <complex>

#include "addon/AddonHelpers.hpp"
#include "audio/WavProcessor.hpp"
#include "audio/WindowFunctions.hpp"
#include "audio/FFTProcessor.hpp"

namespace {

std::vector<double> toDoubleVector(const Napi::Env &env, const Napi::Array &array,
                                   const char *context) {
    std::vector<double> values(array.Length());
    for (uint32_t i = 0; i < array.Length(); ++i) {
        Napi::Value v = array.Get(i);
        if (!v.IsNumber()) {
            throw Napi::TypeError::New(env, std::string(context) + " must contain only numbers");
        }
        values[i] = v.As<Napi::Number>().DoubleValue();
    }
    return values;
}

std::vector<std::complex<double>> toComplexVector(const Napi::Env &env, const Napi::Array &array,
                                                  const char *context) {
    std::vector<std::complex<double>> values(array.Length());
    for (uint32_t i = 0; i < array.Length(); ++i) {
        Napi::Value v = array.Get(i);
        if (!v.IsObject()) {
            throw Napi::TypeError::New(env, std::string(context) +
                                                " entries must be objects with real/imag");
        }
        Napi::Object bin = v.As<Napi::Object>();
        if (!bin.Has("real") || !bin.Has("imag")) {
            throw Napi::TypeError::New(env, std::string(context) +
                                                " entries must have real and imag fields");
        }
        values[i] = std::complex<double>(bin.Get("real").As<Napi::Number>().DoubleValue(),
                                         bin.Get("imag").As<Napi::Number>().DoubleValue());
    }
    return values;
}

Napi::Array vectorToNumberArray(const Napi::Env &env, const std::vector<double> &values) {
    Napi::Array result = Napi::Array::New(env, values.size());
    for (size_t i = 0; i < values.size(); ++i) {
        result[i] = Napi::Number::New(env, values[i]);
    }
    return result;
}

Napi::Array complexVectorToArray(const Napi::Env &env,
                                 const std::vector<std::complex<double>> &spectrum) {
    Napi::Array result = Napi::Array::New(env, spectrum.size());
    for (size_t i = 0; i < spectrum.size(); ++i) {
        Napi::Object bin = Napi::Object::New(env);
        bin.Set("real", Napi::Number::New(env, spectrum[i].real()));
        bin.Set("imag", Napi::Number::New(env, spectrum[i].imag()));
        result[i] = bin;
    }
    return result;
}

Napi::Value GenerateWindow(const Napi::CallbackInfo &info) {
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
        return vectorToNumberArray(env, window);
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value ApplyWindow(const Napi::CallbackInfo &info) {
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
        std::vector<double> samples = toDoubleVector(env, samplesArray, "samples");
        std::vector<double> window = toDoubleVector(env, windowArray, "window");

        WindowFunctions::apply(samples, window);
        return vectorToNumberArray(env, samples);
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value FFT(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsArray()) {
        throw Napi::TypeError::New(env, "Expected (samples: number[])");
    }

    Napi::Array samplesArray = info[0].As<Napi::Array>();
    if (samplesArray.Length() == 0) {
        throw Napi::TypeError::New(env, "Input array cannot be empty");
    }

    try {
        std::vector<double> samples = toDoubleVector(env, samplesArray, "samples");
        const std::vector<std::complex<double>> spectrum = FFTProcessor::fft(samples);
        return complexVectorToArray(env, spectrum);
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value IFFT(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsArray()) {
        throw Napi::TypeError::New(env, "Expected (spectrum: {real:number, imag:number}[])");
    }

    Napi::Array spectrumArray = info[0].As<Napi::Array>();
    if (spectrumArray.Length() == 0) {
        throw Napi::TypeError::New(env, "Spectrum array cannot be empty");
    }

    try {
        std::vector<std::complex<double>> spectrum =
            toComplexVector(env, spectrumArray, "Spectrum");
        const std::vector<double> samples = FFTProcessor::ifft(spectrum);
        return vectorToNumberArray(env, samples);
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value FFTMagnitude(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsArray()) {
        throw Napi::TypeError::New(env, "Expected (spectrum: {real:number, imag:number}[])");
    }

    Napi::Array spectrumArray = info[0].As<Napi::Array>();
    try {
        std::vector<std::complex<double>> spectrum =
            toComplexVector(env, spectrumArray, "Spectrum");
        const std::vector<double> mags = FFTProcessor::magnitude(spectrum);
        return vectorToNumberArray(env, mags);
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Value Process(const Napi::CallbackInfo &info) {
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
        result.Set("fftApplied", Napi::Boolean::New(env, processed.fftApplied));
        result.Set("fftFramesProcessed", Napi::Number::New(env, processed.fftFramesProcessed));
        result.Set("fftBins", Napi::Number::New(env, processed.fftBins));
        result.Set("fftCutoffBin", Napi::Number::New(env, processed.fftCutoffBin));
        return result;
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, e.what());
    }
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("process", Napi::Function::New(env, Process));
    exports.Set("generateWindow", Napi::Function::New(env, GenerateWindow));
    exports.Set("applyWindow", Napi::Function::New(env, ApplyWindow));
    exports.Set("fft", Napi::Function::New(env, FFT));
    exports.Set("ifft", Napi::Function::New(env, IFFT));
    exports.Set("fftMagnitude", Napi::Function::New(env, FFTMagnitude));
    return exports;
}

} // namespace

NODE_API_MODULE(dissonance_core, Init)
