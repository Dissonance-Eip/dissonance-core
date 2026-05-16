#pragma once

#include <stdexcept>

namespace dissonance {

struct WavFormatError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct DspError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

} // namespace dissonance
