#include "effects/GateEffect.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <spdlog/spdlog.h>

GateEffect::GateEffect(float x, float y, const std::string& assetPath)
    : x(x), y(y), active(true), finished(false),
      animationTimer(0.0f), frameDuration(0.15f), currentFrame(0), totalFrames(12),
      gateTextureID(0), textureWidth(0), textureHeight(0),
      frameWidth(64), frameHeight(64), framesPerRow(12), totalRows(9), looping(true) {
    loadGateTexture(assetPath);
}

GateEffect::~GateEffect() {
    cleanupTexture();
}

void GateEffect::loadGateTexture(const std::string& assetPath) {
    std::string gateFile = assetPath + "assets/graphic/enviroment/gate_effects.png";
    
    spdlog::info("Attempting to load gate effect texture: {}", gateFile);
    
    int width, height, channels;
    unsigned char* data = stbi_load(gateFile.c_str(), &width, &height, &channels, 0);
    if (!data) {
        spdlog::error("Failed to load gate effect texture: {}", gateFile);
        spdlog::error("STB Error: {}", stbi_failure_reason());
        gateTextureID = 0;
        return;
    }
    
    textureWidth = width;
    textureHeight = height;
    
    // Frame dimensions are fixed at 64x64 as specified
    // frameWidth and frameHeight are already set to 64 in constructor
    
    spdlog::info("Loaded gate effect texture: {} ({}x{}, channels: {}, frames: {}x{}, frame size: {}x{})", 
                 gateFile, width, height, channels, framesPerRow, totalRows, frameWidth, frameHeight);
    
    glGenTextures(1, &gateTextureID);
    glBindTexture(GL_TEXTURE_2D, gateTextureID);
    
    // Use NEAREST filtering for pixel-perfect graphics
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    GLenum format = GL_RGB;
    if (channels == 4) format = GL_RGBA;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 1) format = GL_RED;
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    
    spdlog::info("Gate effect texture uploaded to OpenGL successfully, textureID: {}", gateTextureID);
}

void GateEffect::cleanupTexture() {
    if (gateTextureID != 0) {
        glDeleteTextures(1, &gateTextureID);
        gateTextureID = 0;
    }
}

void GateEffect::update(float deltaTime) {
    if (!active || finished || gateTextureID == 0) return;
    
    animationTimer += deltaTime;
    if (animationTimer >= frameDuration) {
        animationTimer -= frameDuration;
        currentFrame++;
        
        if (currentFrame >= totalFrames) {
            if (looping) {
                // Loop back to the beginning
                currentFrame = 0;
            } else {
                // Animation finished
                finished = true;
                active = false;
            }
        }
    }
}

void GateEffect::draw() const {
    if (!active || finished || gateTextureID == 0) {
        return;
    }
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gateTextureID);
    
    // Calculate which frame to draw (use second row, cycle through all columns)
    int frameX = currentFrame % framesPerRow; // Cycle through all 12 columns
    int frameY = 1; // Second row (0-indexed: row 0 = first, row 1 = second)
    
    // Calculate UV coordinates for the current frame
    float u1 = (float)(frameX * frameWidth) / textureWidth;
    float v1 = (float)(frameY * frameHeight) / textureHeight;
    float u2 = (float)((frameX + 1) * frameWidth) / textureWidth;
    float v2 = (float)((frameY + 1) * frameHeight) / textureHeight;
    
    // Draw gate effect centered on the gate position (original size)
    float drawX = x - frameWidth / 2.0f;
    float drawY = y - frameHeight / 2.0f;
    
    glBegin(GL_QUADS);
    glTexCoord2f(u1, v1); glVertex2f(drawX, drawY);
    glTexCoord2f(u2, v1); glVertex2f(drawX + frameWidth, drawY);
    glTexCoord2f(u2, v2); glVertex2f(drawX + frameWidth, drawY + frameHeight);
    glTexCoord2f(u1, v2); glVertex2f(drawX, drawY + frameHeight);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
}
