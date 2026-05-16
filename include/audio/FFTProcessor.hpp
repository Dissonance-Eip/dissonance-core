#pragma once

#include <complex>
#include <vector>

namespace fft {

std::vector<std::complex<double>> transform(const std::vector<float> &input);
std::vector<double> inverse(const std::vector<std::complex<double>> &spectrum);
std::vector<double> magnitude(const std::vector<std::complex<double>> &spectrum);

} // namespace fft
