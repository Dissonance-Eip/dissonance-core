/**
 * @file addon.cpp
 * @brief Node.js native addon entry point.
 *
 * Exports two functions to JavaScript:
 *   process(inputPath)      – runs the full audio protection pipeline
 *                             asynchronously, resolves { ok, processedPath }.
 *   readMetadata(inputPath) – reads WAV header and LIST/INFO tags without
 *                             decoding audio, resolves { ok, audio, tags }.
 */

#include <fstream>

#include <napi.h>

#include "audio/WavProcessor.hpp"
#include "utils/WavParser.hpp"
#include "utils/WavUtils.hpp"

namespace {

// ---------------------------------------------------------------------------
// process()
// ---------------------------------------------------------------------------

class ProcessWorker : public Napi::AsyncWorker {
  public:
    ProcessWorker(Napi::Env env, std::string inputPath, ProcessingOptions opts,
                  Napi::Promise::Deferred deferred)
        : Napi::AsyncWorker(env), inputPath_(std::move(inputPath)), opts_(std::move(opts)),
          deferred_(std::move(deferred)) {}

    void Execute() override {
        try {
            result_ = processWavFile(inputPath_, opts_);
        } catch (const std::exception &e) {
            SetError(e.what());
        }
    }

    void OnOK() override {
        Napi::Object result = Napi::Object::New(Env());
        result.Set("ok", Napi::Boolean::New(Env(), true));
        result.Set("processedPath", Napi::String::New(Env(), result_.processedPath));
        deferred_.Resolve(result);
    }

    void OnError(const Napi::Error &e) override { deferred_.Reject(e.Value()); }

  private:
    std::string inputPath_;
    ProcessingOptions opts_;
    ProcessedWav result_;
    Napi::Promise::Deferred deferred_;
};

// process(inputPath: string, options?: { outputPath?: string, perturbation?: number })
//   inputPath              – WAV file to process (required)
//   options.outputPath     – optional destination. Defaults to
//                            <inputDir>/<stem>-processed.wav (CLI / tests).
//                            The UI always supplies a temp path.
//   options.perturbation   – perturbation strength in [0, 1]. Defaults to the
//                            ProcessingOptions default if omitted.
Napi::Value Process(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString())
        throw Napi::TypeError::New(env, "Input path must be a string");

    ProcessingOptions opts;
    if (info.Length() >= 2 && info[1].IsObject()) {
        Napi::Object jsOpts = info[1].As<Napi::Object>();

        if (jsOpts.Has("outputPath")) {
            Napi::Value v = jsOpts.Get("outputPath");
            if (v.IsString())
                opts.outputPath = v.As<Napi::String>().Utf8Value();
        }
        if (jsOpts.Has("perturbation")) {
            Napi::Value v = jsOpts.Get("perturbation");
            if (v.IsNumber())
                opts.perturbation = static_cast<float>(v.As<Napi::Number>().DoubleValue());
        }
    }

    auto deferred = Napi::Promise::Deferred::New(env);
    (new ProcessWorker(env, info[0].As<Napi::String>(), std::move(opts), deferred))->Queue();
    return deferred.Promise();
}

// ---------------------------------------------------------------------------
// readMetadata()
// ---------------------------------------------------------------------------

class ReadMetadataWorker : public Napi::AsyncWorker {
  public:
    ReadMetadataWorker(Napi::Env env, std::string inputPath, Napi::Promise::Deferred deferred)
        : Napi::AsyncWorker(env), inputPath_(std::move(inputPath)), deferred_(std::move(deferred)) {
    }

    void Execute() override {
        try {
            std::ifstream file(inputPath_, std::ios::binary);
            if (!file.is_open())
                throw std::runtime_error("Failed to open file: " + inputPath_);

            // readAudioData = false — header only, no sample decode.
            parser_ = Parser::fromFile(file, false);

            const auto &chunks = parser_.getOtherChunks();
            if (auto it = chunks.find("LIST"); it != chunks.end())
                tags_ = parseListChunk(it->second);
        } catch (const std::exception &e) {
            SetError(e.what());
        }
    }

    void OnOK() override {
        Napi::Env env = Env();

        Napi::Object audio = Napi::Object::New(env);
        audio.Set("audioFormat", Napi::Number::New(env, parser_.getAudioFormat()));
        audio.Set("numChannels", Napi::Number::New(env, parser_.getNumChannels()));
        audio.Set("sampleRate", Napi::Number::New(env, parser_.getSampleRate()));
        audio.Set("byteRate", Napi::Number::New(env, parser_.getByteRate()));
        audio.Set("blockAlign", Napi::Number::New(env, parser_.getBlockAlign()));
        audio.Set("bitsPerSample", Napi::Number::New(env, parser_.getBitsPerSample()));

        // Derive sample count from the data chunk size.
        const uint32_t dataSize = parser_.getSubchunk2Size();
        const uint16_t bps = parser_.getBitsPerSample();
        const uint16_t channels = parser_.getNumChannels();
        const uint32_t rate = parser_.getSampleRate();
        const uint32_t bpsBytes = bps > 0 ? bps / 8 : 0;
        const double durationSec =
            (bpsBytes > 0 && channels > 0 && rate > 0)
                ? static_cast<double>(dataSize) / (bpsBytes * channels * rate)
                : 0.0;
        audio.Set("durationSec", Napi::Number::New(env, durationSec));

        Napi::Object tags = Napi::Object::New(env);
        tags.Set("title", Napi::String::New(env, tags_.title));
        tags.Set("artist", Napi::String::New(env, tags_.artist));
        tags.Set("comment", Napi::String::New(env, tags_.comment));
        tags.Set("date", Napi::String::New(env, tags_.date));
        tags.Set("genre", Napi::String::New(env, tags_.genre));
        tags.Set("software", Napi::String::New(env, tags_.software));
        tags.Set("copyright", Napi::String::New(env, tags_.copyright));

        Napi::Object result = Napi::Object::New(env);
        result.Set("ok", Napi::Boolean::New(env, true));
        result.Set("audio", audio);
        result.Set("tags", tags);
        deferred_.Resolve(result);
    }

    void OnError(const Napi::Error &e) override { deferred_.Reject(e.Value()); }

  private:
    std::string inputPath_;
    Parser parser_;
    ListTags tags_;
    Napi::Promise::Deferred deferred_;
};

Napi::Value ReadMetadata(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString())
        throw Napi::TypeError::New(env, "Input path must be a string");

    auto deferred = Napi::Promise::Deferred::New(env);
    (new ReadMetadataWorker(env, info[0].As<Napi::String>(), deferred))->Queue();
    return deferred.Promise();
}

// ---------------------------------------------------------------------------
// writeTags()
// ---------------------------------------------------------------------------

class WriteTagsWorker : public Napi::AsyncWorker {
  public:
    WriteTagsWorker(Napi::Env env, std::string filePath, ListTags tags,
                    Napi::Promise::Deferred deferred)
        : Napi::AsyncWorker(env), filePath_(std::move(filePath)), tags_(std::move(tags)),
          deferred_(std::move(deferred)) {}

    void Execute() override {
        try {
            writeTagsToWav(filePath_, tags_);
        } catch (const std::exception &e) {
            SetError(e.what());
        }
    }

    void OnOK() override {
        Napi::Object result = Napi::Object::New(Env());
        result.Set("ok", Napi::Boolean::New(Env(), true));
        deferred_.Resolve(result);
    }

    void OnError(const Napi::Error &e) override { deferred_.Reject(e.Value()); }

  private:
    std::string filePath_;
    ListTags tags_;
    Napi::Promise::Deferred deferred_;
};

Napi::Value WriteTags(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsObject())
        throw Napi::TypeError::New(env, "Expected (filePath: string, tags: object)");

    const std::string filePath = info[0].As<Napi::String>();
    Napi::Object obj = info[1].As<Napi::Object>();

    auto str = [&](const char *key) -> std::string {
        if (!obj.Has(key))
            return {};
        Napi::Value v = obj.Get(key);
        return v.IsString() ? v.As<Napi::String>().Utf8Value() : std::string{};
    };

    ListTags tags;
    tags.title = str("title");
    tags.artist = str("artist");
    tags.comment = str("comment");
    tags.date = str("date");
    tags.genre = str("genre");
    tags.software = str("software");
    tags.copyright = str("copyright");

    auto deferred = Napi::Promise::Deferred::New(env);
    (new WriteTagsWorker(env, filePath, std::move(tags), deferred))->Queue();
    return deferred.Promise();
}

// ---------------------------------------------------------------------------

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("process", Napi::Function::New(env, Process));
    exports.Set("readMetadata", Napi::Function::New(env, ReadMetadata));
    exports.Set("writeTags", Napi::Function::New(env, WriteTags));
    return exports;
}

} // namespace

NODE_API_MODULE(dissonance_core, Init)
