//
// Created by noe on 16/03/2025.
//

#include "cli/WavGUI.hpp"
#include "utils/WavUtils.hpp"

#include <sstream>

const std::array<std::string, 5> COLORS = {
    "\033[38;5;214m", // Orange
    "\033[38;5;226m", // Yellow
    "\033[38;5;196m", // Red
    "\033[38;5;46m",  // Green
    "\033[0m"         // Reset
};

void printField(const std::string &label, const std::string &value) {
    if (!value.empty()) {
        std::cout << COLORS[0] << label << ": " << COLORS[1] << value << COLORS[4] << std::endl;
    }
}

GUI::GUI(const std::string &filename)
    : processed(processWavFile(filename, 0.8)), valid(true), filename(filename) {
    std::cout << COLORS[0] << "Opening file: " << COLORS[3] << filename << COLORS[4] << std::endl;
    std::cout << COLORS[3] << "File opened successfully" << COLORS[4] << std::endl;
}

void GUI::printMetadata() const {
    if (!valid) {
        throw std::runtime_error("Failed to read file data");
    }
    // Print formatted metadata with colors
    const std::string metadata = processed.metadataText;
    std::istringstream stream(metadata);
    std::string line;
    while (std::getline(stream, line)) {
        const size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::cout << COLORS[0] << line.substr(0, colonPos + 1) << COLORS[1]
                      << line.substr(colonPos + 1) << COLORS[4] << std::endl;
        } else {
            std::cout << line << std::endl;
        }
    }
}

void GUI::printAudioData() const {
    if (valid) {
        const auto &audioData = processed.originalSamples;
        std::ofstream outFile("../audio_data.bin", std::ios::binary);
        if (!outFile) {
            throw std::runtime_error("Failed to open output file");
        }
        outFile.write(reinterpret_cast<const char *>(audioData.data()),
                      audioData.size() * sizeof(int16_t));
        outFile.close();
        std::cout << "Audio data saved to audio_data.bin" << std::endl;
    } else {
        throw std::runtime_error("Failed to read audio data");
    }
}

void GUI::printWaveform() const {
    const std::string wave = renderWaveformASCII(processed.originalSamples);
    for (const char ch : wave) {
        if (ch == '|') {
            std::cout << COLORS[3] << ch << COLORS[4];
        } else {
            std::cout << ch;
        }
    }
}

void GUI::printOtherChunks() const {
    if (valid) {
        const auto &otherChunks = processed.otherChunks;
        for (const auto &[key, value] : otherChunks) {
            if (key == "LIST") {
                printListChunk(value);
            } else {
                printGenericChunk(key, value);
            }
        }
    } else {
        throw std::runtime_error("Failed to read other chunks");
    }
}

void GUI::printListChunk(const std::vector<char> &value) const {
    std::cout << std::endl << COLORS[3] << "MetaData:" << COLORS[4] << std::endl;
    const ListTags tags = parseListChunk(value);

    // Store in mutable fields
    title = tags.title;
    name = tags.artist;
    description = tags.comment;
    date = tags.date;
    software = tags.software;
    genre = tags.genre;
    copyright = tags.copyright;

    printField("Title", title);
    printField("Date", date);
    printField("Name", name);
    printField("Description", description);
    printField("Software", software);
    printField("Genre", genre);
    printField("Copyright", copyright);
}

void GUI::printGenericChunk(const std::string &key, const std::vector<char> &value) {
    std::cout << "Chunk " << key << " data:" << std::endl;
    for (size_t i = 0; i < value.size(); ++i) {
        const char ch = value[i];
        std::cout << (std::isprint(ch) ? ch : '.');
        if ((i + 1) % 16 == 0 || i == value.size() - 1) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}
