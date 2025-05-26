//
// Created by noe on 16/03/2025.
//

#include <iostream>
#include <memory>
#include <stdexcept>
#include "WavGUI.hpp"

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            throw std::runtime_error("Usage: " + std::string(argv[0]) + " <path_to_wav_file>");
        }

        if (const auto parser = std::make_shared<GUI>(argv[1]); parser->isValid()) {
            parser->printMetadata();
            parser->printOtherChunks();
            parser->printWaveform();
        } else {
            throw std::runtime_error("WAV file invalid.");
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}