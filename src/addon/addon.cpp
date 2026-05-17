/**
 * @file addon.cpp
 * @brief Node.js native addon entry point.
 *
 * Exposes a single `process(inputPath)` function to JavaScript that runs the
 * full audio protection pipeline asynchronously and resolves with
 * { ok, processedPath }.
 */

#include <napi.h>

#include "audio/WavProcessor.hpp"

namespace {

class ProcessWorker : public Napi::AsyncWorker {
  public:
    ProcessWorker(Napi::Env env, std::string inputPath, Napi::Promise::Deferred deferred)
        : Napi::AsyncWorker(env), inputPath_(std::move(inputPath)), deferred_(std::move(deferred)) {
    }

    void Execute() override {
        try {
            result_ = processWavFile(inputPath_);
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
    ProcessedWav result_;
    Napi::Promise::Deferred deferred_;
};

Napi::Value Process(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString())
        throw Napi::TypeError::New(env, "Input path must be a string");

    auto deferred = Napi::Promise::Deferred::New(env);
    (new ProcessWorker(env, info[0].As<Napi::String>(), deferred))->Queue();
    return deferred.Promise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("process", Napi::Function::New(env, Process));
    return exports;
}

} // namespace

NODE_API_MODULE(dissonance_core, Init)
