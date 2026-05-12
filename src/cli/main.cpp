#include <iostream>
#include <string>
#include <cstdlib>
#include "cli/Commands.hpp"

int main(int argc, char **argv) {
    try {
        if (argc < 3) {
            Commands::printUsage(argv[0]);
            return EXIT_FAILURE;
        }

        std::string command = argv[1];
        std::string inputPath = argv[2];

        if (command == "info") {
            return Commands::handleInfo(inputPath);
        }

        if (command == "process") {
            return Commands::handleProcess(inputPath, argc, argv, 3);
        }

        if (command == "fft") {
            return Commands::handleFft(inputPath, argc, argv, 3);
        }

        std::cerr << "Unknown command: " << command << "\n";
        Commands::printUsage(argv[0]);
        return EXIT_FAILURE;

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
