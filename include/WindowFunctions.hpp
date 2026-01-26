#ifndef WINDOWFUNCTIONS_HPP
#define WINDOWFUNCTIONS_HPP

#include <vector>
#include <cstddef>

class WindowFunctions {
public:
    enum class Type {
        Hann,
        Hamming
    };

    /**
     * Generate a window function of the specified type and size
     * @param type The window type (Hann or Hamming)
     * @param size The number of samples in the window
     * @return Vector containing the window coefficients (values between 0 and 1)
     */
    static std::vector<double> generate(Type type, size_t size);

    /**
     * Apply a window function to a buffer of samples
     * @param samples The audio samples to window (modified in place)
     * @param window The window coefficients to apply
     */
    static void apply(std::vector<double>& samples, const std::vector<double>& window);

    /**
     * Apply a window function to int16_t samples
     * @param samples The audio samples to window (modified in place)
     * @param window The window coefficients to apply
     */
    static void apply(std::vector<int16_t>& samples, const std::vector<double>& window);

private:
    static std::vector<double> generateHann(size_t size);
    static std::vector<double> generateHamming(size_t size);
};

#endif // WINDOWFUNCTIONS_HPP
