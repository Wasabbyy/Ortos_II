#include "ui/RomanNumeralRenderer.h"
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <GLFW/glfw3.h>

RomanNumeralRenderer::RomanNumeralRenderer() : isInitialized(false) {
}

RomanNumeralRenderer::~RomanNumeralRenderer() {
    cleanup();
}

bool RomanNumeralRenderer::initialize(const std::string& assetPath) {
    if (isInitialized) {
        spdlog::warn("RomanNumeralRenderer already initialized");
        return true;
    }
    
    spdlog::info("Initializing RomanNumeralRenderer with path: {}", assetPath);
    
    // Load each Roman numeral texture
    bool success = true;
    success &= loadNumeralTexture(assetPath + "/numeral_I.png", 'I');
    success &= loadNumeralTexture(assetPath + "/numeral_V.png", 'V');
    success &= loadNumeralTexture(assetPath + "/numeral_X.png", 'X');
    success &= loadNumeralTexture(assetPath + "/numeral_C.png", 'C');
    
    if (success) {
        isInitialized = true;
        spdlog::info("RomanNumeralRenderer initialized successfully");
    } else {
        spdlog::error("Failed to initialize RomanNumeralRenderer");
        cleanup();
    }
    
    return success;
}

bool RomanNumeralRenderer::loadNumeralTexture(const std::string& filepath, char symbol) {
    int width, height, channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    
    if (!data) {
        spdlog::error("Failed to load Roman numeral texture: {}", filepath);
        return false;
    }
    
    spdlog::info("Loaded Roman numeral '{}': {} ({}x{}, {} channels)", 
                 symbol, filepath, width, height, channels);
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Use NEAREST filtering to maintain pixel art look
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    GLenum format = GL_RGBA;
    if (channels == 4) format = GL_RGBA;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 1) format = GL_RED;
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    
    NumeralTexture tex;
    tex.textureID = textureID;
    tex.width = width;
    tex.height = height;
    
    numeralTextures[symbol] = tex;
    
    return true;
}

std::string RomanNumeralRenderer::toRomanNumeral(int number) {
    if (number < 1 || number > 100) {
        spdlog::warn("Number {} out of range for Roman numerals (1-100)", number);
        return "?";
    }
    
    std::string result = "";
    
    // Roman numeral conversion table
    struct RomanPair {
        int value;
        const char* numeral;
    };
    
    const RomanPair romanTable[] = {
        {100, "C"},
        {90, "XC"},
        {50, "L"},
        {40, "XL"},
        {10, "X"},
        {9, "IX"},
        {5, "V"},
        {4, "IV"},
        {1, "I"}
    };
    
    for (const auto& pair : romanTable) {
        while (number >= pair.value) {
            result += pair.numeral;
            number -= pair.value;
        }
    }
    
    return result;
}

void RomanNumeralRenderer::drawRomanNumeral(int number, float x, float y, float scale) {
    if (!isInitialized) {
        spdlog::warn("RomanNumeralRenderer not initialized, cannot draw numeral");
        return;
    }
    
    std::string romanStr = toRomanNumeral(number);
    if (romanStr == "?") {
        return;
    }
    
    float currentX = x;
    
    // Enable blending for transparent backgrounds
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Draw each symbol
    for (size_t i = 0; i < romanStr.length(); i++) {
        char symbol = romanStr[i];
        
        // Handle multi-character sequences (like "XC", "IX", etc.)
        // For rendering, we need to check if next char is part of a subtractive notation
        if (i < romanStr.length() - 1) {
            char nextSymbol = romanStr[i + 1];
            // Check if this is a subtractive pair (like IV, IX, XL, XC)
            if ((symbol == 'I' && (nextSymbol == 'V' || nextSymbol == 'X')) ||
                (symbol == 'X' && (nextSymbol == 'L' || nextSymbol == 'C'))) {
                // Draw both symbols close together for subtractive notation
                drawSymbol(symbol, currentX, y, scale);
                auto it = numeralTextures.find(symbol);
                if (it != numeralTextures.end()) {
                    currentX += it->second.width * scale * 0.6f; // Closer spacing
                }
                drawSymbol(nextSymbol, currentX, y, scale);
                it = numeralTextures.find(nextSymbol);
                if (it != numeralTextures.end()) {
                    currentX += it->second.width * scale + 2.0f * scale; // Small gap
                }
                i++; // Skip next symbol since we already drew it
                continue;
            }
        }
        
        // Regular symbol drawing
        drawSymbol(symbol, currentX, y, scale);
        auto it = numeralTextures.find(symbol);
        if (it != numeralTextures.end()) {
            currentX += it->second.width * scale + 2.0f * scale; // Small gap between symbols
        }
    }
}

void RomanNumeralRenderer::drawSymbol(char symbol, float x, float y, float scale) {
    auto it = numeralTextures.find(symbol);
    if (it == numeralTextures.end()) {
        spdlog::warn("Roman numeral symbol '{}' not found", symbol);
        return;
    }
    
    const NumeralTexture& tex = it->second;
    
    float width = tex.width * scale;
    float height = tex.height * scale;
    
    glBindTexture(GL_TEXTURE_2D, tex.textureID);
    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y + height);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + height);
    glEnd();
}

float RomanNumeralRenderer::getRomanNumeralWidth(int number, float scale) const {
    if (!isInitialized) {
        return 0.0f;
    }
    
    std::string romanStr = toRomanNumeral(number);
    if (romanStr == "?") {
        return 0.0f;
    }
    
    float totalWidth = 0.0f;
    
    for (size_t i = 0; i < romanStr.length(); i++) {
        char symbol = romanStr[i];
        auto it = numeralTextures.find(symbol);
        if (it != numeralTextures.end()) {
            // Check for subtractive notation pairs
            if (i < romanStr.length() - 1) {
                char nextSymbol = romanStr[i + 1];
                if ((symbol == 'I' && (nextSymbol == 'V' || nextSymbol == 'X')) ||
                    (symbol == 'X' && (nextSymbol == 'L' || nextSymbol == 'C'))) {
                    // Add both symbols with tight spacing
                    totalWidth += it->second.width * scale * 0.6f;
                    auto nextIt = numeralTextures.find(nextSymbol);
                    if (nextIt != numeralTextures.end()) {
                        totalWidth += nextIt->second.width * scale + 2.0f * scale;
                    }
                    i++; // Skip next symbol
                    continue;
                }
            }
            totalWidth += it->second.width * scale + 2.0f * scale;
        }
    }
    
    return totalWidth;
}

void RomanNumeralRenderer::cleanup() {
    for (auto& pair : numeralTextures) {
        glDeleteTextures(1, &pair.second.textureID);
    }
    numeralTextures.clear();
    isInitialized = false;
    spdlog::info("RomanNumeralRenderer cleaned up");
}

