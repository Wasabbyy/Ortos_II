#include "effects/BloodEffect.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <spdlog/spdlog.h>

BloodEffect::BloodEffect(float x, float y)
    : x(x), y(y), active(true), finished(false),
      animationTimer(0.0f), frameDuration(0.1f), currentFrame(0), totalFrames(5),
      textureWidth(0), textureHeight(0) {
    loadBloodTextures();
    spdlog::debug("Blood effect created at position ({}, {})", x, y);
}

BloodEffect::~BloodEffect() {
    cleanupTextures();
}

void BloodEffect::loadBloodTextures() {
    std::vector<std::string> bloodFiles = {
        "../assets/graphic/blood/blood_01.png",
        "../assets/graphic/blood/blood_02.png",
        "../assets/graphic/blood/blood_03.png",
        "../assets/graphic/blood/blood_04.png",
        "../assets/graphic/blood/blood_05.png"
    };
    
    bloodTextures.resize(totalFrames);
    
    for (int i = 0; i < totalFrames; ++i) {
        int width, height, channels;
        unsigned char* data = stbi_load(bloodFiles[i].c_str(), &width, &height, &channels, 0);
        if (!data) {
            spdlog::error("Failed to load blood texture: {}", bloodFiles[i]);
            bloodTextures[i] = 0;
            continue;
        }
        
        spdlog::debug("Loaded blood texture: {} ({}x{})", bloodFiles[i], width, height);
        
        // Store dimensions from first texture
        if (i == 0) {
            textureWidth = width;
            textureHeight = height;
        }
        
        glGenTextures(1, &bloodTextures[i]);
        glBindTexture(GL_TEXTURE_2D, bloodTextures[i]);
        
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
    }
}

void BloodEffect::cleanupTextures() {
    for (unsigned int textureID : bloodTextures) {
        if (textureID != 0) {
            glDeleteTextures(1, &textureID);
        }
    }
    bloodTextures.clear();
}

void BloodEffect::update(float deltaTime) {
    if (!active || finished) return;
    
    animationTimer += deltaTime;
    if (animationTimer >= frameDuration) {
        animationTimer -= frameDuration;
        currentFrame++;
        
        if (currentFrame >= totalFrames) {
            // Animation finished, keep the last frame (blood_05) on the ground
            currentFrame = totalFrames - 1;
            finished = true;
            spdlog::debug("Blood effect animation finished, keeping blood on ground");
        }
    }
}

void BloodEffect::draw() const {
    if (!active || currentFrame >= bloodTextures.size() || bloodTextures[currentFrame] == 0) return;
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, bloodTextures[currentFrame]);
    
    // Draw blood effect centered on the death position
    float drawX = x - textureWidth / 2.0f;
    float drawY = y - textureHeight / 2.0f;
    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(drawX, drawY);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(drawX + textureWidth, drawY);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(drawX + textureWidth, drawY + textureHeight);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(drawX, drawY + textureHeight);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
} 