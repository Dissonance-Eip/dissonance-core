#include "cli/ConsolePrinter.hpp"
#include "utils/WavUtils.hpp"
#include "core/Errors.hpp"

#include <iostream>
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

ConsolePrinter::ConsolePrinter(const std::string &filename)
    : processed(processWavFile(filename)), valid(true), filename(filename) {
    std::cout << COLORS[0] << "Opening file: " << COLORS[3] << filename << COLORS[4] << std::endl;
    std::cout << COLORS[3] << "File opened successfully" << COLORS[4] << std::endl;
}

void ConsolePrinter::printMetadata() const {
    if (!valid) {
        throw dissonance::WavFormatError("Failed to read file data");
    }
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

void ConsolePrinter::printWaveform() const {
    const std::string wave = renderWaveformASCII(processed.originalSamples);
    for (const char ch : wave) {
        if (ch == '|') {
            std::cout << COLORS[3] << ch << COLORS[4];
        } else {
            std::cout << ch;
        }
    }
}

void ConsolePrinter::printOtherChunks() const {
    if (!valid) {
        throw dissonance::WavFormatError("Failed to read other chunks");
    }
    for (const auto &[key, value] : processed.otherChunks) {
        if (key == "LIST") {
            printListChunk(value);
        } else {
            printGenericChunk(key, value);
        }
    }
}

void ConsolePrinter::printListChunk(const std::vector<char> &value) const {
    std::cout << std::endl << COLORS[3] << "MetaData:" << COLORS[4] << std::endl;
    const ListTags tags = parseListChunk(value);

    printField("Title", tags.title);
    printField("Date", tags.date);
    printField("Name", tags.artist);
    printField("Description", tags.comment);
    printField("Software", tags.software);
    printField("Genre", tags.genre);
    printField("Copyright", tags.copyright);
}

void ConsolePrinter::printGenericChunk(const std::string &key, const std::vector<char> &value) {
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
