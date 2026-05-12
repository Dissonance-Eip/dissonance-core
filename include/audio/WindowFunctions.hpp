#pragma once

#include <cstddef>
#include <vector>

namespace window {

enum class Type { Hann, Hamming };

std::vector<double> generate(Type type, size_t size);
void apply(std::vector<float> &samples, const std::vector<double> &coefficients);

} // namespace window
