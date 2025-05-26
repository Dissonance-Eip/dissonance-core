#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include "../include/WavParser.hpp"
#include "../include/WavGUI.hpp"

// Test that Parser can be constructed and reads a valid file without throwing
TEST(WavParserTest, CanReadValidWavFile) {
    std::ifstream file("../assets/sound.wav", std::ios::binary);
    ASSERT_TRUE(file.is_open()) << "Test WAV file not found: ../assets/sound.wav";

    Parser parser;
    EXPECT_NO_THROW(parser.readFromFile(file));
}

// Test that GUI can be constructed and is valid for a real file
TEST(WavGUITest, GUIValidOnValidFile) {
    ASSERT_NO_THROW({
        GUI gui("../assets/sound.wav");
        EXPECT_TRUE(gui.isValid());
    });
}