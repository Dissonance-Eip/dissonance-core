#pragma once

#include <array>
#include <string>
#include <vector>

#include "../audio/WavProcessor.hpp"

extern const std::array<std::string, 5> COLORS;

void printField(const std::string &label, const std::string &value);

class GUI {
  public:
    explicit GUI(const std::string &filename);
    [[nodiscard]] bool isValid() const { return valid; }
    void printMetadata() const;
    void printOtherChunks() const;
    void printWaveform() const;
    void printListChunk(const std::vector<char> &value) const;
    static void printGenericChunk(const std::string &key, const std::vector<char> &value);

  private:
    ProcessedWav processed;
    bool valid = false;
    std::string filename;
};
