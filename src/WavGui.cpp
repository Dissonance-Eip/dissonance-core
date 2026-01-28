//
// Created by noe on 16/03/2025.
//

#include "WavGUI.hpp"
#include "WavUtils.hpp"

#include <sstream>


GUI::GUI(const std::string& filename) : filename(filename) {
    std::cout << colors[0]<< "Opening file: " << colors[3] <<filename << colors[4] << std::endl;
    processed = processWavFile(filename, 0.8);
    std::cout << colors[3] <<"File opened successfully" << colors[4] << std::endl;
    valid = true;
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
            std::cout << colors[0] << line.substr(0, colonPos + 1) 
                      << colors[1] << line.substr(colonPos + 1) << colors[4] << std::endl;
        } else {
            std::cout << line << std::endl;
        }
    }
}

void GUI::printAudioData() const {
    if (valid) {
        const auto& audioData = processed.originalSamples;
        std::ofstream outFile("../audio_data.bin", std::ios::binary);
        if (!outFile) {
            throw std::runtime_error("Failed to open output file");
        }
        outFile.write(reinterpret_cast<const char*>(audioData.data()), audioData.size() * sizeof(int16_t));
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
            std::cout << colors[3] << ch << colors[4];
        } else {
            std::cout << ch;
        }
    }
}

void GUI::printOtherChunks() const {
    if (valid) {
        const auto& otherChunks = processed.otherChunks;
        for (const auto& [key, value] : otherChunks) {
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

void GUI::printListChunk(const std::vector<char>& value) const {
    std::cout << std::endl << colors[3] << "MetaData:" << colors[4] << std::endl;
    const ListTags tags = parseListChunk(value);
    
    // Store in mutable fields
    title = tags.title;
    name = tags.artist;
    description = tags.comment;
    date = tags.date;
    software = tags.software;
    genre = tags.genre;
    copyright = tags.copyright;

    // Helper to print a field if not empty
    auto printField = [this](const std::string& label, const std::string& value) {
        if (!value.empty()) {
            std::cout << colors[0] << label << ": " << colors[1] << value << colors[4] << std::endl;
        }
    };

    printField("Title", title);
    printField("Date", date);
    printField("Name", name);
    printField("Description", description);
    printField("Software", software);
    printField("Genre", genre);
    printField("Copyright", copyright);
}

void GUI::printGenericChunk(const std::string& key, const std::vector<char>& value) {
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
