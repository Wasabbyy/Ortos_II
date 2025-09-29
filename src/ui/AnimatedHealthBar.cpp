#include "AnimatedHealthBar.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <spdlog/spdlog.h>

AnimatedHealthBar::AnimatedHealthBar()
    : handsTextureID(0), textureWidth(0), textureHeight(0),
      frameWidth(0), frameHeight(0), framesPerRow(0), totalRows(0), totalFrames(0),
      animationTimer(0.0f), frameDuration(0.1f), currentFrame(0),
      barWidth(280.0f), barHeight(22.0f), barX(0.0f), barY(0.0f),
      initialized(false) {
}

AnimatedHealthBar::~AnimatedHealthBar() {
    cleanup();
}

void AnimatedHealthBar::initialize(const std::string& assetPath) {
    if (initialized) return;
    
    loadHandsTexture(assetPath);
    initialized = true;
    spdlog::info("AnimatedHealthBar initialized");
}

void AnimatedHealthBar::loadHandsTexture(const std::string& assetPath) {
    std::string handsFile = assetPath + "assets/graphic/enviroment/hands.png";
    
    spdlog::info("Attempting to load hands texture: {}", handsFile);
    
    int width, height, channels;
    unsigned char* data = stbi_load(handsFile.c_str(), &width, &height, &channels, 0);
    if (!data) {
        spdlog::error("Failed to load hands texture: {}", handsFile);
        spdlog::error("STB Error: {}", stbi_failure_reason());
        handsTextureID = 0;
        return;
    }
    
    textureWidth = width;
    textureHeight = height;
    
    // This is a static image (64x128), not a spritesheet
    framesPerRow = 1;
    totalRows = 1;
    totalFrames = 1;
    
    frameWidth = width;
    frameHeight = height;
    
    spdlog::info("Loaded hands texture: {} ({}x{}, channels: {})", 
                 handsFile, width, height, channels);
    
    glGenTextures(1, &handsTextureID);
    glBindTexture(GL_TEXTURE_2D, handsTextureID);
    
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
    
    spdlog::info("Hands texture uploaded to OpenGL successfully, textureID: {}", handsTextureID);
}

void AnimatedHealthBar::cleanupTexture() {
    if (handsTextureID != 0) {
        glDeleteTextures(1, &handsTextureID);
        handsTextureID = 0;
    }
}

void AnimatedHealthBar::update(float deltaTime) {
    if (!initialized || handsTextureID == 0) return;
    
    animationTimer += deltaTime;
    if (animationTimer >= frameDuration) {
        animationTimer -= frameDuration;
        currentFrame = (currentFrame + 1) % totalFrames;
    }
}

void AnimatedHealthBar::draw(int currentHealth, int maxHealth, int windowWidth, int windowHeight) {
    if (!initialized || handsTextureID == 0 || maxHealth <= 0) return;
    
    // Save current matrix state
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Calculate health ratio
    float healthRatio = static_cast<float>(currentHealth) / maxHealth;
    
    // Set health bar position and make it bigger, moved more to the right
    barX = 170.0f + barWidth / 2.0f; // Moved from 150.0f to 170.0f to move further right
    barY = 40.0f;
    
    // Make the health bar bigger while maintaining proper aspect ratio
    // The hands image is 64x128, so when rotated 90 degrees it becomes 128x64
    // Scale it up proportionally
    float scaleFactor = 4.0f; // Overall scale factor
    float scaledBarWidth = 128.0f * scaleFactor;   // Width after 90-degree rotation
    float scaledBarHeight = 64.0f * scaleFactor;   // Height after 90-degree rotation
    
    // Calculate the position for the rotated and scaled hands
    float handsX = barX - scaledBarWidth / 2.0f;
    float handsY = barY - scaledBarHeight / 2.0f;
    
    // Draw the hands texture rotated 90 degrees and scaled up
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, handsTextureID);
    
    // Draw the hands texture with 90-degree rotation and proper scaling
    // Rotate 90 degrees by swapping UV coordinates and vertices
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    // Rotated 90 degrees: original top-left becomes bottom-left
    glTexCoord2f(0.0f, 1.0f); glVertex2f(handsX, handsY + scaledBarHeight);
    // Original top-right becomes top-left
    glTexCoord2f(1.0f, 1.0f); glVertex2f(handsX + scaledBarWidth, handsY + scaledBarHeight);
    // Original bottom-right becomes top-right
    glTexCoord2f(1.0f, 0.0f); glVertex2f(handsX + scaledBarWidth, handsY);
    // Original bottom-left becomes bottom-right
    glTexCoord2f(0.0f, 0.0f); glVertex2f(handsX, handsY);
    glEnd();
    
    // Now create the health bar effect by masking the red color from right to left
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Calculate how much of the health bar should be "depleted" (from right to left)
    float depletedWidth = scaledBarWidth * (1.0f - healthRatio);
    
    // Draw a red overlay that covers the depleted portion (from right to left)
    if (depletedWidth > 0) {
        glColor4f(0.8f, 0.0f, 0.0f, 0.8f); // Semi-transparent red overlay
        glBegin(GL_QUADS);
        // Cover the rightmost portion that should be "depleted"
        glVertex2f(handsX + scaledBarWidth - depletedWidth, handsY);
        glVertex2f(handsX + scaledBarWidth, handsY);
        glVertex2f(handsX + scaledBarWidth, handsY + scaledBarHeight);
        glVertex2f(handsX + scaledBarWidth - depletedWidth, handsY + scaledBarHeight);
        glEnd();
    }
    
    glDisable(GL_BLEND);
    
    // Draw a subtle border
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(handsX, handsY);
    glVertex2f(handsX + scaledBarWidth, handsY);
    glVertex2f(handsX + scaledBarWidth, handsY + scaledBarHeight);
    glVertex2f(handsX, handsY + scaledBarHeight);
    glEnd();
    
    glColor3f(1.0f, 1.0f, 1.0f);  // Reset color
    
    // Restore matrix state
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    // Re-enable textures
    glEnable(GL_TEXTURE_2D);
}

void AnimatedHealthBar::cleanup() {
    cleanupTexture();
    initialized = false;
}
