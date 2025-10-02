#include "AnimatedXPBar.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <spdlog/spdlog.h>

AnimatedXPBar::AnimatedXPBar()
    : textureWidth(0), textureHeight(0),
      frameWidth(0), frameHeight(0),
      barWidth(300.0f), barHeight(20.0f), barX(0.0f), barY(0.0f),
      initialized(false) {
    // Initialize all texture IDs to 0
    for (int i = 0; i < 5; i++) {
        xpTextures[i] = 0;
    }
}

AnimatedXPBar::~AnimatedXPBar() {
    cleanup();
}

void AnimatedXPBar::initialize(const std::string& assetPath) {
    if (initialized) return;
    
    loadXPTextures(assetPath);
    initialized = true;
    spdlog::info("AnimatedXPBar initialized with XP bar sprites");
}

void AnimatedXPBar::loadXPTextures(const std::string& assetPath) {
    // XP states: xpbar_01 to xpbar_05
    for (int i = 0; i < 5; i++) {
        std::string xpFile = assetPath + "assets/graphic/enviroment/xpbar/split/xpbar_" + 
                            std::to_string(i + 1).insert(0, 2 - std::to_string(i + 1).length(), '0') + ".png";
        
        spdlog::info("Attempting to load XP texture: {}", xpFile);
        
        int width, height, channels;
        unsigned char* data = stbi_load(xpFile.c_str(), &width, &height, &channels, 4); // Force 4 channels (RGBA)
        if (!data) {
            spdlog::error("Failed to load XP texture: {}", xpFile);
            spdlog::error("STB Error: {}", stbi_failure_reason());
            xpTextures[i] = 0;
            continue;
        }
        
        // Set texture dimensions from first loaded texture
        if (i == 0) {
            textureWidth = width;
            textureHeight = height;
            frameWidth = width;
            frameHeight = height;
        }
        
        spdlog::info("Loaded XP texture: {} ({}x{}, channels: {})", 
                     xpFile, width, height, channels);
        
        glGenTextures(1, &xpTextures[i]);
        glBindTexture(GL_TEXTURE_2D, xpTextures[i]);
        
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
        
        spdlog::info("XP texture {} uploaded to OpenGL successfully, textureID: {}", 
                     i + 1, xpTextures[i]);
    }
}

void AnimatedXPBar::cleanupTextures() {
    for (int i = 0; i < 5; i++) {
        if (xpTextures[i] != 0) {
            glDeleteTextures(1, &xpTextures[i]);
            xpTextures[i] = 0;
        }
    }
}

int AnimatedXPBar::getXPStateIndex(int currentXP, int maxXP) {
    if (maxXP <= 0) return 0; // Return first state if no max XP
    
    float xpRatio = static_cast<float>(currentXP) / maxXP;
    
    // Map XP ratio to state index (0-4) with thresholds
    // Each state represents 20% of the XP bar
    if (xpRatio >= 0.8f) return 4;      // 80-100% -> xpbar_05
    else if (xpRatio >= 0.6f) return 3; // 60-80%  -> xpbar_04
    else if (xpRatio >= 0.4f) return 2; // 40-60%  -> xpbar_03
    else if (xpRatio >= 0.2f) return 1; // 20-40%  -> xpbar_02
    else return 0;                      // 0-20%   -> xpbar_01
}

void AnimatedXPBar::update(float deltaTime) {
    // No animation needed for static XP sprites
    // This function is kept for compatibility but does nothing
}

void AnimatedXPBar::draw(int currentXP, int maxXP, int windowWidth, int windowHeight) {
    if (!initialized || maxXP <= 0) return;
    
    // Get the appropriate XP sprite index
    int xpIndex = getXPStateIndex(currentXP, maxXP);
    drawWithState(xpIndex, windowWidth, windowHeight);
}

void AnimatedXPBar::drawWithState(int xpState, int windowWidth, int windowHeight) {
    if (!initialized || xpState < 0 || xpState >= 5) return;
    
    if (xpTextures[xpState] == 0) return;
    
    // Save current matrix state
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Set XP bar position (same as original XP bar)
    barX = 960.0f;  // Center horizontally
    barY = 55.0f;    // Top of screen
    
    // Use the natural texture dimensions with a scale factor for bigger size
    float scaleFactor = 5.0f;  // 🔧 ADJUST THIS VALUE TO MAKE BIGGER/SMALLER (1.0 = normal, 2.0 = double, 3.0 = triple, etc.)
    float scaledBarWidth = static_cast<float>(textureWidth) * scaleFactor;
    float scaledBarHeight = static_cast<float>(textureHeight) * scaleFactor;
    
    // Calculate the position for the XP bar
    float xpBarX = barX - scaledBarWidth / 2.0f;
    float xpBarY = barY - scaledBarHeight / 2.0f;
    
    // Draw the appropriate XP sprite
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);  // Enable blending for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Standard alpha blending
    glBindTexture(GL_TEXTURE_2D, xpTextures[xpState]);
    
    // Draw the XP sprite
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // Use Color4f to include alpha channel
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(xpBarX, xpBarY);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(xpBarX + scaledBarWidth, xpBarY);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(xpBarX + scaledBarWidth, xpBarY + scaledBarHeight);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(xpBarX, xpBarY + scaledBarHeight);
    glEnd();
    
    // Disable blending and texture
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    
    // Restore matrix state
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void AnimatedXPBar::cleanup() {
    cleanupTextures();
    initialized = false;
}
