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

std::vector<std::complex<double>> transform(const std::vector<double> &input) {
    if (input.empty())
        throw dissonance::DspError("FFT input cannot be empty");

    const int N = static_cast<int>(input.size());
    KissCfg cfg(N, 0);

    std::vector<kiss_fft_cpx> in(N), out(N);
    for (int i = 0; i < N; ++i) {
        in[i].r = static_cast<float>(input[i]);
        in[i].i = 0.0f;
    }

    kiss_fft(cfg.cfg, in.data(), out.data());

    std::vector<std::complex<double>> result(N);
    for (int i = 0; i < N; ++i)
        result[i] = {static_cast<double>(out[i].r), static_cast<double>(out[i].i)};
    return result;
}

std::vector<double> inverse(const std::vector<std::complex<double>> &spectrum) {
    if (spectrum.empty())
        throw dissonance::DspError("iFFT input cannot be empty");

    const int N = static_cast<int>(spectrum.size());
    KissCfg cfg(N, 1);

    std::vector<kiss_fft_cpx> in(N), out(N);
    for (int i = 0; i < N; ++i) {
        in[i].r = static_cast<float>(spectrum[i].real());
        in[i].i = static_cast<float>(spectrum[i].imag());
    }

    kiss_fft(cfg.cfg, in.data(), out.data());

    const double scale = 1.0 / static_cast<double>(N);
    std::vector<double> result(N);
    for (int i = 0; i < N; ++i)
        result[i] = static_cast<double>(out[i].r) * scale;
    return result;
}

std::vector<double> magnitude(const std::vector<std::complex<double>> &spectrum) {
    std::vector<double> mags(spectrum.size());
    for (size_t i = 0; i < spectrum.size(); ++i)
        mags[i] = std::abs(spectrum[i]);
    return mags;
}

} // namespace fft
