#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include "../include/WavParser.hpp"
#include "../include/WavGUI.hpp"

// Test that Parser can be constructed and reads a valid file without throwing
TEST(WavParserTest, CanReadValidWavFile) {
    std::string testFile = "../assets/sound.wav";
    std::ifstream file(testFile, std::ios::binary);
    ASSERT_TRUE(file.is_open()) << "Test WAV file not found: " << testFile;

    Parser parser;
    EXPECT_NO_THROW(parser.readFromFile(file));
}

// Test that GUI can be constructed and is valid for a real file
TEST(WavGUITest, GUIValidOnValidFile) {
    std::string testFile = "../assets/sound.wav";
    ASSERT_NO_THROW({
        GUI gui(testFile);
        EXPECT_TRUE(gui.isValid());
    });
}
