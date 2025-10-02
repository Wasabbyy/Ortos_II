#include "AnimatedHealthBar.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <spdlog/spdlog.h>

AnimatedHealthBar::AnimatedHealthBar()
    : textureWidth(0), textureHeight(0),
      frameWidth(0), frameHeight(0),
      barWidth(280.0f), barHeight(22.0f), barX(0.0f), barY(0.0f),
      initialized(false) {
    // Initialize all texture IDs to 0
    for (int i = 0; i < 6; i++) {
        healthTextures[i] = 0;
    }
}

AnimatedHealthBar::~AnimatedHealthBar() {
    cleanup();
}

void AnimatedHealthBar::initialize(const std::string& assetPath) {
    if (initialized) return;
    
    loadHealthTextures(assetPath);
    initialized = true;
    spdlog::info("AnimatedHealthBar initialized with detailed health sprites");
}

void AnimatedHealthBar::loadHealthTextures(const std::string& assetPath) {
    // Health levels: 100%, 80%, 60%, 40%, 20%, 0%
    int healthPercentages[] = {100, 80, 60, 40, 20, 0};
    
    for (int i = 0; i < 6; i++) {
        std::string healthFile = assetPath + "assets/graphic/enviroment/healbar/detailed/hands_health_" + 
                                std::to_string(healthPercentages[i]) + ".png";
        
        spdlog::info("Attempting to load health texture: {}", healthFile);
        
        int width, height, channels;
        unsigned char* data = stbi_load(healthFile.c_str(), &width, &height, &channels, 4); // Force 4 channels (RGBA)
        if (!data) {
            spdlog::error("Failed to load health texture: {}", healthFile);
            spdlog::error("STB Error: {}", stbi_failure_reason());
            healthTextures[i] = 0;
            continue;
        }
        
        // Set texture dimensions from first loaded texture
        if (i == 0) {
            textureWidth = width;
            textureHeight = height;
            frameWidth = width;
            frameHeight = height;
        }
        
        spdlog::info("Loaded health texture: {} ({}x{}, channels: {})", 
                     healthFile, width, height, channels);
        
        glGenTextures(1, &healthTextures[i]);
        glBindTexture(GL_TEXTURE_2D, healthTextures[i]);
        
        // Use NEAREST filtering for pixel-perfect graphics
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Always use RGBA format since we forced 4 channels
        GLenum format = GL_RGBA;
        GLenum internalFormat = GL_RGBA;
        
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        
        spdlog::info("Health texture {} uploaded to OpenGL successfully, textureID: {}", 
                     healthPercentages[i], healthTextures[i]);
    }
}

void AnimatedHealthBar::cleanupTextures() {
    for (int i = 0; i < 6; i++) {
        if (healthTextures[i] != 0) {
            glDeleteTextures(1, &healthTextures[i]);
            healthTextures[i] = 0;
        }
    }
}

int AnimatedHealthBar::getHealthLevelIndex(int currentHealth, int maxHealth) {
    if (maxHealth <= 0) return 5; // Return 0% health index
    
    float healthRatio = static_cast<float>(currentHealth) / maxHealth;
    
    // Map health ratio to index (0-5) with exact thresholds
    if (healthRatio >= 1.0f) return 0;      // 100%
    else if (healthRatio >= 0.8f) return 1; // 80%
    else if (healthRatio >= 0.6f) return 2; // 60%
    else if (healthRatio >= 0.4f) return 3; // 40%
    else if (healthRatio >= 0.2f) return 4; // 20%
    else return 5;                          // 0%
}

void AnimatedHealthBar::update(float deltaTime) {
    // No animation needed for static health sprites
    // This function is kept for compatibility but does nothing
}

void AnimatedHealthBar::draw(int currentHealth, int maxHealth, int windowWidth, int windowHeight) {
    if (!initialized || maxHealth <= 0) return;
    
    // Get the appropriate health sprite index
    int healthIndex = getHealthLevelIndex(currentHealth, maxHealth);
    if (healthTextures[healthIndex] == 0) return;
    
    // Save current matrix state
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Set health bar position and make it bigger, moved more to the right
    barX = 80.0f + barWidth / 2.0f; // Moved from 150.0f to 170.0f to move further right
    barY = 70.0f;
    
    // Make the health bar smaller while maintaining proper aspect ratio
    // The hands image is 128x64, so when rotated 90 degrees it becomes 64x128
    // Scale it up proportionally
    float scaleFactor = 3.0f; // Reduced scale factor for smaller health bar
    float scaledBarWidth = 128.0f * scaleFactor;   // Width after 90-degree rotation
    float scaledBarHeight = 64.0f * scaleFactor;   // Height after 90-degree rotation
    
    // Calculate the position for the rotated and scaled hands
    float handsX = barX - scaledBarWidth / 2.0f;
    float handsY = barY - scaledBarHeight / 2.0f;
    
    // Draw the appropriate health sprite rotated 90 degrees and scaled up
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);  // Enable blending for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Standard alpha blending
    glBindTexture(GL_TEXTURE_2D, healthTextures[healthIndex]);
    
    // Draw the health sprite with 90-degree rotation and proper scaling
    // Rotate 90 degrees by swapping UV coordinates and vertices
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // Use Color4f to include alpha channel
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
    
    // Draw a subtle border
    glDisable(GL_TEXTURE_2D);
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
    
    // Re-enable textures and disable blending
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);  // Disable blending to avoid affecting other rendering
}

void AnimatedHealthBar::cleanup() {
    cleanupTextures();
    initialized = false;
}
