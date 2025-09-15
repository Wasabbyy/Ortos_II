#pragma once
#include <GLFW/glfw3.h>
#include "ft2build.h"
#include "freetype/freetype.h"
#include <map>
#include <string>
#include <glm/glm.hpp>

struct Character {
    GLuint textureID;   // ID handle of the glyph texture
    glm::ivec2 size;    // Size of glyph
    glm::ivec2 bearing; // Offset from baseline to left/top of glyph
    GLuint advance;     // Horizontal offset to advance to next glyph
};

class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();
    
    // Initialize the text renderer with a font file
    bool init(const std::string& fontPath, unsigned int fontSize = 16);
    
    // Render text
    void renderText(const std::string& text, float x, float y, float scale, 
                   float r = 1.0f, float g = 1.0f, float b = 1.0f);
    
    // Get text width for centering calculations
    float getTextWidth(const std::string& text, float scale = 1.0f);
    
    // Get text height
    float getTextHeight(float scale = 1.0f);
    
    // Cleanup
    void cleanup();

private:
    std::map<GLchar, Character> characters;
    GLuint VBO;
    GLuint shaderProgram;
    FT_Library ft;
    FT_Face face;
    bool initialized;
    
    // Helper functions
    bool compileShaders();
    void setupBuffers();
}; 