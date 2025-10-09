#pragma once
#include <string>
#include <map>
#include <GLFW/glfw3.h>

class RomanNumeralRenderer {
public:
    RomanNumeralRenderer();
    ~RomanNumeralRenderer();
    
    // Initialize and load Roman numeral textures
    bool initialize(const std::string& assetPath);
    
    // Convert number to Roman numeral string (supports 1-100)
    static std::string toRomanNumeral(int number);
    
    // Draw Roman numeral at specified position
    void drawRomanNumeral(int number, float x, float y, float scale = 1.0f);
    
    // Get width of a Roman numeral rendering (for positioning)
    float getRomanNumeralWidth(int number, float scale = 1.0f) const;
    
    // Cleanup
    void cleanup();
    
private:
    struct NumeralTexture {
        GLuint textureID;
        int width;
        int height;
    };
    
    std::map<char, NumeralTexture> numeralTextures;  // I, V, X, C
    bool isInitialized;
    
    // Load a single numeral texture
    bool loadNumeralTexture(const std::string& filepath, char symbol);
    
    // Draw a single numeral symbol at position
    void drawSymbol(char symbol, float x, float y, float scale);
};

