#include "TextRenderer.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

TextRenderer::TextRenderer() : initialized(false), VBO(0), shaderProgram(0) {
    // Initialize FreeType
    if (FT_Init_FreeType(&ft)) {
        spdlog::error("ERROR::FREETYPE: Could not init FreeType Library");
        return;
    }
}

TextRenderer::~TextRenderer() {
    cleanup();
    if (ft) {
        FT_Done_FreeType(ft);
    }
}

bool TextRenderer::init(const std::string& fontPath, unsigned int fontSize) {
    spdlog::info("TextRenderer::init - Loading font: {} with size: {}", fontPath, fontSize);
    
    // Load font face
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        spdlog::error("ERROR::FREETYPE: Failed to load font: {}", fontPath);
        return false;
    }
    
    spdlog::info("Font loaded successfully");
    
    // Set font size
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    
    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    // Load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++) {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            spdlog::warn("ERROR::FREETYPE: Failed to load Glyph for character: {}", c);
            continue;
        }
        
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // Prepare a two-channel buffer for GL_LUMINANCE_ALPHA
        std::vector<unsigned char> buffer(face->glyph->bitmap.width * face->glyph->bitmap.rows * 2);
        for (int i = 0; i < face->glyph->bitmap.width * face->glyph->bitmap.rows; ++i) {
            buffer[2 * i] = 255; // Luminance (white)
            buffer[2 * i + 1] = face->glyph->bitmap.buffer[i]; // Alpha
        }
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_LUMINANCE_ALPHA,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_LUMINANCE_ALPHA,
            GL_UNSIGNED_BYTE,
            buffer.data()
        );
        
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };
        characters.insert(std::pair<char, Character>(c, character));
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    
    spdlog::info("TextRenderer initialized successfully with {} characters", characters.size());
    initialized = true;
    return true;
}

bool TextRenderer::compileShaders() {
    // For legacy OpenGL, we'll use a simple approach without shaders
    // We'll render text using immediate mode OpenGL
    return true;
}

void TextRenderer::setupBuffers() {
    // For legacy OpenGL, we don't need VAOs or VBOs
    // We'll use immediate mode rendering
}

void TextRenderer::renderText(const std::string& text, float x, float y, float scale, float r, float g, float b) {
    if (!initialized) {
        spdlog::warn("TextRenderer not initialized!");
        return;
    }
    
    spdlog::debug("Rendering text: '{}' at ({}, {}) scale={}", text, x, y, scale);
    
    // Activate corresponding render state
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set text color
    glColor3f(r, g, b);
    
    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        if (characters.find(*c) == characters.end()) {
            spdlog::warn("Character '{}' not found in font", *c);
            continue;
        }
        
        Character ch = characters[*c];
        
        GLfloat xpos = x + ch.bearing.x * scale;
        GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;
        
        GLfloat w = ch.size.x * scale;
        GLfloat h = ch.size.y * scale;
        
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        
        // Draw the character
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(xpos, ypos + h);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(xpos, ypos);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(xpos + w, ypos);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(xpos + w, ypos + h);
        glEnd();
        
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    
    glColor3f(1.0f, 1.0f, 1.0f); // Reset color
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

float TextRenderer::getTextWidth(const std::string& text, float scale) {
    if (!initialized) return 0.0f;
    
    float width = 0.0f;
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = characters[*c];
        width += (ch.advance >> 6) * scale;
    }
    return width;
}

float TextRenderer::getTextHeight(float scale) {
    if (!initialized || characters.empty()) return 0.0f;
    
    // Use the height of the first character as a reference
    Character ch = characters.begin()->second;
    return ch.size.y * scale;
}

void TextRenderer::cleanup() {
    if (initialized) {
        // Delete character textures
        for (auto& pair : characters) {
            glDeleteTextures(1, &pair.second.textureID);
        }
        characters.clear();
        
        // Delete OpenGL objects
        if (VBO) glDeleteBuffers(1, &VBO);
        if (shaderProgram) glDeleteProgram(shaderProgram);
        
        initialized = false;
    }
} 