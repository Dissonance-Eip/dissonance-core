#pragma once

#include <vector>

class GainProcessor {
  public:
    explicit GainProcessor(double gain) : gain_(gain) {}

    void apply(std::vector<float> &samples) const;
    [[nodiscard]] double gain() const { return gain_; }

  private:
    double gain_;
};
