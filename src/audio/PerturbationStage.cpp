/**
 * @file PerturbationStage.cpp
 * @brief Band-limited high-frequency noise injection for AI voice-clone protection.
 */

#include "audio/PerturbationStage.hpp"

#include <algorithm>
#include <cmath>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PerturbationStage::PerturbationStage(float strength, uint32_t sampleRate, uint64_t seed)
    : strength_(std::clamp(strength, 0.0f, 1.0f)), sampleRate_(sampleRate), seed_(seed) {}

void PerturbationStage::process(std::vector<float> &samples, uint16_t numChannels) {
    if (strength_ <= 0.0f || samples.empty() || numChannels == 0)
        return;

    // First-order high-pass IIR: y[n] = alpha * (y[n-1] + x[n] - x[n-1])
    // alpha = tau / (tau + Ts),  tau = 1 / (2 * pi * cutoffHz)
    const float Ts = 1.0f / static_cast<float>(sampleRate_);
    const float tau = 1.0f / (2.0f * static_cast<float>(M_PI) * kCutoffHz);
    const float alpha = tau / (tau + Ts);

    const float noiseAmplitude = strength_ * kMaxAmplitude;
    const size_t totalSamples = samples.size();

    // Generate white noise
    std::mt19937 rng(static_cast<uint32_t>(seed_ ^ (seed_ >> 32)));
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    std::vector<float> noise(totalSamples);
    for (auto &n : noise)
        n = dist(rng) * noiseAmplitude;

    // Apply HP filter independently per channel (interleaved layout)
    const size_t framesCount = totalSamples / static_cast<size_t>(numChannels);
    for (uint16_t ch = 0; ch < numChannels; ++ch) {
        float prevIn = 0.0f;
        float prevOut = 0.0f;
        for (size_t frame = 0; frame < framesCount; ++frame) {
            const size_t idx = frame * numChannels + ch;
            const float x = noise[idx];
            const float y = alpha * (prevOut + x - prevIn);
            prevIn = x;
            prevOut = y;
            noise[idx] = y;
        }
    }

    // Mix noise into signal and measure actual RMS
    double sumSq = 0.0;
    for (size_t i = 0; i < totalSamples; ++i) {
        sumSq += static_cast<double>(noise[i]) * static_cast<double>(noise[i]);
        samples[i] = std::clamp(samples[i] + noise[i], -1.0f, 1.0f);
    }

    const double rms = std::sqrt(sumSq / static_cast<double>(totalSamples));
    rmsDbfs_ = (rms > 0.0) ? static_cast<float>(20.0 * std::log10(rms)) : kSilentDbfs;
}
