#ifndef GAINPROCESSOR_HPP
#define GAINPROCESSOR_HPP

#include <cstdint>
#include <vector>

class GainProcessor {
  public:
    explicit GainProcessor(double gain) : gain_(gain) {}

    void apply(std::vector<int16_t> &samples) const;
    [[nodiscard]] double gain() const { return gain_; }

  private:
    double gain_;
};

#endif // GAINPROCESSOR_HPP
