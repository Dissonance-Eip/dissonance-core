#pragma once

#include <stdexcept>

namespace dissonance {

/** @brief Thrown when a WAV file is missing, corrupt, or in an unsupported format. */
struct WavFormatError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/** @brief Thrown when a DSP operation fails (e.g. FFT allocation, invalid parameters). */
struct DspError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

} // namespace dissonance
