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

#include "WavParser.hpp"

class GUI {
public:
    explicit GUI(const std::string& filename);
    [[nodiscard]] bool isValid() const { return valid; }
    void printMetadata() const;
    void printAudioData() const;
    void printOtherChunks() const;
    void printWaveform() const;
    void printListChunk(const std::vector<char>& value) const;
    static void printGenericChunk(const std::string& key, const std::vector<char>& value) ;

private:
    std::shared_ptr<Parser> data;
    bool valid = false;
    std::string filename;
    std::array<std::string, 5> colors = {
        "\033[38;5;214m", // Orange
        "\033[38;5;226m", // Yellow
        "\033[38;5;196m",  // Red
        "\033[38;5;46m",  // Green
        "\033[0m" // Reset
    };
    mutable std::string title;
    mutable std::string date;
    mutable std::string name;
    mutable std::string description;
    mutable std::string software;
    mutable std::string genre;
    mutable std::string copyright;
};

#endif // WAVGUI_H