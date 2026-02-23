//
// Created by noe on 16/03/2025.
//

#ifndef WAVGUI_H
#define WAVGUI_H

#include <memory>
#include <string>
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>

#include "../audio/WavProcessor.hpp"

// Color codes for console output
extern const std::array<std::string, 5> COLORS;

// Utility function for colored field printing
void printField(const std::string &label, const std::string &value);

class GUI {
  public:
    explicit GUI(const std::string &filename);
    [[nodiscard]] bool isValid() const { return valid; }
    void printMetadata() const;
    void printAudioData() const;
    void printOtherChunks() const;
    void printWaveform() const;
    void printListChunk(const std::vector<char> &value) const;
    static void printGenericChunk(const std::string &key, const std::vector<char> &value);

  private:
    ProcessedWav processed;
    bool valid = false;
    std::string filename;
    mutable std::string title;
    mutable std::string date;
    mutable std::string name;
    mutable std::string description;
    mutable std::string software;
    mutable std::string genre;
    mutable std::string copyright;
};

#endif // WAVGUI_H
