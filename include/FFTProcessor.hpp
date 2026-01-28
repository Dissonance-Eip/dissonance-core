#ifndef FFTPROCESSOR_HPP
#define FFTPROCESSOR_HPP

#include <complex>
#include <vector>

class FFTProcessor {
public:
    /**
     * Compute the discrete Fourier transform (DFT) of a real-valued signal.
     * @throws std::invalid_argument if input is empty.
     */
    static std::vector<std::complex<double>> fft(const std::vector<double>& input);

    /**
     * Compute the inverse DFT of a complex spectrum and return real samples.
     * @throws std::invalid_argument if input is empty.
     */
    static std::vector<double> ifft(const std::vector<std::complex<double>>& spectrum);

    /**
     * Compute magnitude of each complex bin.
     */
    static std::vector<double> magnitude(const std::vector<std::complex<double>>& spectrum);
};

#endif // FFTPROCESSOR_HPP
