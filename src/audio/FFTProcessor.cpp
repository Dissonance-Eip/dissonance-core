/**
 * @file FFTProcessor.cpp
 * @brief KissFFT-backed float FFT implementation.
 *
 * Thin wrapper around kiss_fft that manages config allocation/deallocation via
 * RAII (KissCfg) and exposes forward transform, normalised inverse, and
 * magnitude helpers — all operating on float precision.
 */

#include "audio/FFTProcessor.hpp"
#include "core/Errors.hpp"

#include <kiss_fft.h>

namespace {

struct KissCfg {
    kiss_fft_cfg cfg;
    KissCfg(int n, int inverse) : cfg(kiss_fft_alloc(n, inverse, nullptr, nullptr)) {
        if (!cfg)
            throw dissonance::DspError("Failed to allocate KissFFT config");
    }
    ~KissCfg() { kiss_fft_free(cfg); }
    KissCfg(const KissCfg &) = delete;
    KissCfg &operator=(const KissCfg &) = delete;
};

} // namespace

namespace fft {

std::vector<std::complex<float>> transform(const std::vector<float> &input) {
    if (input.empty())
        throw dissonance::DspError("FFT input cannot be empty");

    const int N = static_cast<int>(input.size());
    KissCfg cfg(N, 0);

    std::vector<kiss_fft_cpx> in(N), out(N);
    for (int i = 0; i < N; ++i) {
        in[i].r = input[i];
        in[i].i = 0.0f;
    }

    kiss_fft(cfg.cfg, in.data(), out.data());

    std::vector<std::complex<float>> result(N);
    for (int i = 0; i < N; ++i)
        result[i] = {out[i].r, out[i].i};
    return result;
}

std::vector<float> inverse(const std::vector<std::complex<float>> &spectrum) {
    if (spectrum.empty())
        throw dissonance::DspError("iFFT input cannot be empty");

    const int N = static_cast<int>(spectrum.size());
    KissCfg cfg(N, 1);

    std::vector<kiss_fft_cpx> in(N), out(N);
    for (int i = 0; i < N; ++i) {
        in[i].r = spectrum[i].real();
        in[i].i = spectrum[i].imag();
    }

    kiss_fft(cfg.cfg, in.data(), out.data());

    const float scale = 1.0f / static_cast<float>(N);
    std::vector<float> result(N);
    for (int i = 0; i < N; ++i)
        result[i] = out[i].r * scale;
    return result;
}

std::vector<float> magnitude(const std::vector<std::complex<float>> &spectrum) {
    std::vector<float> mags(spectrum.size());
    for (size_t i = 0; i < spectrum.size(); ++i)
        mags[i] = std::abs(spectrum[i]);
    return mags;
}

} // namespace fft
