#pragma once

#include <string>

class Commands {
  public:
    static void printUsage(const char *programName);
    static int handleInfo(const std::string &inputPath);
    static int handleProcess(const std::string &inputPath, int argc, char **argv, int startIdx);
    static int handleFft(const std::string &inputPath);
};
