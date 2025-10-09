#include "ui/UI.h"
#include <cmath>
#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stb_image.h>

// Static member initialization
TextRenderer* UI::textRenderer = nullptr;
bool UI::initialized = false;
GLuint UI::titleScreenTextureID = 0;
GLuint UI::deathScreenTextureID = 0;
AnimatedHealthBar* UI::animatedHealthBar = nullptr;
AnimatedXPBar* UI::animatedXPBar = nullptr;
RomanNumeralRenderer* UI::romanNumeralRenderer = nullptr;

bool UI::init(const std::string& fontPath) {
    if (initialized) {
        spdlog::warn("UI already initialized!");
        return true;
    }
    
    spdlog::info("Initializing UI system with font: {}", fontPath);
    textRenderer = new TextRenderer();
    // Try provided path first (larger default pixel size for clarity)
    if (!textRenderer->init(fontPath, 28)) {
        spdlog::warn("Primary font path failed: {}. Trying fallbacks...", fontPath);
        // Fallbacks: absolute project root + relative, and explicit pixel font
        const std::string projectRoot = "/Users/filipstupar/Documents/OrtosII/";
        const std::string relFallback = projectRoot + fontPath;
        const std::string pixelFallback = projectRoot + "assets/fonts/pixel.ttf";

        if (!textRenderer->init(relFallback, 28)) {
            spdlog::warn("Relative-to-root font path failed: {}", relFallback);
            if (!textRenderer->init(pixelFallback, 28)) {
                spdlog::error("Failed to initialize TextRenderer with all font fallbacks. Last tried: {}", pixelFallback);
                delete textRenderer;
                textRenderer = nullptr;
                return false;
            } else {
                spdlog::info("Loaded font via explicit fallback: {}", pixelFallback);
            }
        } else {
            spdlog::info("Loaded font via project-root-relative path: {}", relFallback);
        }
    }
    
    initialized = true;
    spdlog::info("UI system initialized successfully with textRenderer: {}", (textRenderer != nullptr));
    return true;
}

bool UI::loadTitleScreenTexture(const std::string& imagePath) {
    int width, height, channels;
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        spdlog::error("Failed to load title screen texture: {}", imagePath);
        return false;
    }
    
    spdlog::info("Loaded title screen texture: {} ({}x{})", imagePath, width, height);
    
    glGenTextures(1, &titleScreenTextureID);
    glBindTexture(GL_TEXTURE_2D, titleScreenTextureID);
    
    // Use LINEAR filtering for smooth scaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    GLenum format = GL_RGB;
    if (channels == 4) format = GL_RGBA;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 1) format = GL_RED;
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    
    spdlog::debug("Title screen texture loaded successfully with ID: {}", titleScreenTextureID);
    return true;
}

bool UI::loadDeathScreenTexture(const std::string& imagePath) {
    int width, height, channels;
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        spdlog::error("Failed to load death screen texture: {}", imagePath);
        return false;
    }
    
    spdlog::info("Loaded death screen texture: {} ({}x{})", imagePath, width, height);
    
    glGenTextures(1, &deathScreenTextureID);
    glBindTexture(GL_TEXTURE_2D, deathScreenTextureID);
    
    // Use LINEAR filtering for smooth scaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    GLenum format = GL_RGB;
    if (channels == 4) format = GL_RGBA;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 1) format = GL_RED;
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    
    spdlog::debug("Death screen texture loaded successfully with ID: {}", deathScreenTextureID);
    return true;
}

void UI::cleanup() {
    if (textRenderer) {
        textRenderer->cleanup();
        delete textRenderer;
        textRenderer = nullptr;
    }
    
    if (titleScreenTextureID != 0) {
        glDeleteTextures(1, &titleScreenTextureID);
        titleScreenTextureID = 0;
    }
    
    if (deathScreenTextureID != 0) {
        glDeleteTextures(1, &deathScreenTextureID);
        deathScreenTextureID = 0;
    }
    
    if (animatedHealthBar) {
        animatedHealthBar->cleanup();
        delete animatedHealthBar;
        animatedHealthBar = nullptr;
    }
    
    if (animatedXPBar) {
        animatedXPBar->cleanup();
        delete animatedXPBar;
        animatedXPBar = nullptr;
    }
    
    if (romanNumeralRenderer) {
        romanNumeralRenderer->cleanup();
        delete romanNumeralRenderer;
        romanNumeralRenderer = nullptr;
    }
    
    initialized = false;
}

void UI::initAnimatedHealthBar(const std::string& assetPath) {
    if (animatedHealthBar) {
        spdlog::warn("AnimatedHealthBar already initialized!");
        return;
    }
    
    animatedHealthBar = new AnimatedHealthBar();
    animatedHealthBar->initialize(assetPath);
    spdlog::info("AnimatedHealthBar initialized");
}

void UI::updateAnimatedHealthBar(float deltaTime) {
    if (animatedHealthBar) {
        animatedHealthBar->update(deltaTime);
    }
}

void UI::initAnimatedXPBar(const std::string& assetPath) {
    if (animatedXPBar) {
        spdlog::warn("AnimatedXPBar already initialized!");
        return;
    }
    
    animatedXPBar = new AnimatedXPBar();
    animatedXPBar->initialize(assetPath);
    spdlog::info("AnimatedXPBar initialized");
}

void UI::updateAnimatedXPBar(float deltaTime) {
    if (animatedXPBar) {
        animatedXPBar->update(deltaTime);
    }
}

void UI::initRomanNumeralRenderer(const std::string& assetPath) {
    if (romanNumeralRenderer) {
        spdlog::warn("RomanNumeralRenderer already initialized!");
        return;
    }
    
    romanNumeralRenderer = new RomanNumeralRenderer();
    romanNumeralRenderer->initialize(assetPath);
    spdlog::info("RomanNumeralRenderer initialized");
}

void UI::drawAnimatedPlayerHealth(int currentHealth, int maxHealth, int windowWidth, int windowHeight) {
    if (animatedHealthBar) {
        animatedHealthBar->draw(currentHealth, maxHealth, windowWidth, windowHeight);
    } else {
        // Fallback to regular health bar if animated one is not available
        drawPlayerHealth(currentHealth, maxHealth, windowWidth, windowHeight);
    }
}

void UI::drawAnimatedXPBar(int currentXP, int maxXP, int windowWidth, int windowHeight) {
    if (animatedXPBar) {
        animatedXPBar->draw(currentXP, maxXP, windowWidth, windowHeight);
    } else {
        // Fallback to regular XP bar if animated one is not available
        drawXPBar(currentXP, maxXP, windowWidth, windowHeight);
    }
}

void UI::drawAnimatedXPBarWithState(int xpState, int windowWidth, int windowHeight) {
    if (animatedXPBar) {
        animatedXPBar->drawWithState(xpState, windowWidth, windowHeight);
    } else {
        // Fallback to regular XP bar if animated one is not available
        // Use a default XP amount based on state
        int currentXP = xpState * 20; // Each state represents 20 XP
        int maxXP = 100;
        drawXPBar(currentXP, maxXP, windowWidth, windowHeight);
    }
}

void UI::drawText(const std::string& text, float x, float y, float scale, float r, float g, float b) {
    if (!initialized || !textRenderer) {
        return;
    }
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1920, 0, 1080, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(r, g, b);
    textRenderer->renderText(text, x, y, scale, r, g, b);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UI::drawCenteredText(const std::string& text, float x, float y, float scale, float r, float g, float b) {
    if (!initialized || !textRenderer) {
        return;
    }
    float textWidth = textRenderer->getTextWidth(text, scale);
    float centeredX = x - textWidth / 2.0f;
    drawText(text, centeredX, y, scale, r, g, b);
}

void UI::drawPlayerHealth(int currentHealth, int maxHealth, int windowWidth, int windowHeight) {
    
    // Save current matrix state
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Disable textures for UI drawing
    glDisable(GL_TEXTURE_2D);
    
    float barWidth = 280.0f;
    float barHeight = 22.0f;
    float x = 20.0f + barWidth / 2.0f;
    float y = 40.0f;
    float healthRatio = (maxHealth > 0) ? static_cast<float>(currentHealth) / maxHealth : 0.0f;

    // Draw background (darker red)
    glColor3f(0.6f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(x - barWidth/2, y - barHeight/2);
    glVertex2f(x + barWidth/2, y - barHeight/2);
    glVertex2f(x + barWidth/2, y + barHeight/2);
    glVertex2f(x - barWidth/2, y + barHeight/2);
    glEnd();

    // Draw health (brighter green)
    if (healthRatio > 0) {
        glColor3f(0.2f, 1.0f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(x - barWidth/2, y - barHeight/2);
        glVertex2f(x - barWidth/2 + barWidth * healthRatio, y - barHeight/2);
        glVertex2f(x - barWidth/2 + barWidth * healthRatio, y + barHeight/2);
        glVertex2f(x - barWidth/2, y + barHeight/2);
        glEnd();
    }

    // Draw border (darker)
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x - barWidth/2, y - barHeight/2);
    glVertex2f(x + barWidth/2, y - barHeight/2);
    glVertex2f(x + barWidth/2, y + barHeight/2);
    glVertex2f(x - barWidth/2, y + barHeight/2);
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

void UI::drawXPBar(int currentXP, int maxXP, int windowWidth, int windowHeight) {
    
    // Save current matrix state
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Disable textures for UI drawing
    glDisable(GL_TEXTURE_2D);
    
    float barWidth = 300.0f;
    float barHeight = 20.0f;
    float x = 1700.0f;  // Center horizontally
    float y = 30.0f;  // Top of screen
    float xpRatio = (maxXP > 0) ? static_cast<float>(currentXP) / maxXP : 0.0f;

    // Draw background (dark blue)
    glColor3f(0.0f, 0.0f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(x - barWidth/2, y - barHeight/2);
    glVertex2f(x + barWidth/2, y - barHeight/2);
    glVertex2f(x + barWidth/2, y + barHeight/2);
    glVertex2f(x - barWidth/2, y + barHeight/2);
    glEnd();

    // Draw XP (bright blue)
    if (xpRatio > 0) {
        glColor3f(0.0f, 0.5f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(x - barWidth/2, y - barHeight/2);
        glVertex2f(x - barWidth/2 + barWidth * xpRatio, y - barHeight/2);
        glVertex2f(x - barWidth/2 + barWidth * xpRatio, y + barHeight/2);
        glVertex2f(x - barWidth/2, y + barHeight/2);
        glEnd();
    }

    // Draw border (white)
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x - barWidth/2, y - barHeight/2);
    glVertex2f(x + barWidth/2, y - barHeight/2);
    glVertex2f(x + barWidth/2, y + barHeight/2);
    glVertex2f(x - barWidth/2, y + barHeight/2);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);  // Reset color
    
    // Draw XP text
    if (textRenderer) {
        std::string xpText = std::to_string(currentXP) + "/" + std::to_string(maxXP);
        // Use the same text rendering setup as the level indicator
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 1920, 0, 1080, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor3f(1.0f, 1.0f, 1.0f);
        float xpTextX = 1700.0f;  // X position for XP text
        float xpTextY = 1000.0f;  // Y position for XP text
        textRenderer->renderText(xpText, xpTextX, xpTextY, 0.6f, 1.0f, 1.0f, 1.0f);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
 
    // Restore matrix state
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    // Re-enable textures
    glEnable(GL_TEXTURE_2D);
}

void UI::drawLevelIndicator(int level, int windowWidth, int windowHeight) {
    
    // Save current matrix state
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);

    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Position next to health bar at the top of the screen
    float x = 80.0f;  // Health bar width + margin
    float y = windowHeight - 150.0f;  // Top of screen
    
    // Level background removed - just draw text
    glColor3f(1.0f, 1.0f, 1.0f);  // Reset color
    glEnable(GL_TEXTURE_2D);
    
    // Draw "Level" text first
    if (textRenderer) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        textRenderer->renderText("Level", x, y, 0.8f, 1.0f, 1.0f, 1.0f);
    }
    
    // Draw Roman numeral using pixel art if available
    if (romanNumeralRenderer) {
        // Position Roman numeral after "Level" text
        float numeralX = x + 80.0f;  // Offset for "Level " text
        float numeralY = y - 35.0f;  // Slightly below the text
        float scale = 0.4f;  // Scale for better readability
        
        romanNumeralRenderer->drawRomanNumeral(level, numeralX, numeralY, scale);
    } else {
        // Fallback to text-based Roman numerals if renderer not initialized
        std::string romanStr = RomanNumeralRenderer::toRomanNumeral(level);
        if (textRenderer) {
            textRenderer->renderText(romanStr, x + 90.0f, y, 0.8f, 1.0f, 1.0f, 1.0f);
        }
    }
    
    // Restore matrix state
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UI::drawEnemyHealthBar(float x, float y, int currentHealth, int maxHealth) {
    if (maxHealth <= 0) return;
    
    float barWidth = 32.0f;
    float barHeight = 4.0f;
    float healthRatio = static_cast<float>(currentHealth) / maxHealth;
    
    // Draw background (red)
    glColor3f(0.8f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(x - barWidth/2, y - barHeight/2);
    glVertex2f(x + barWidth/2, y - barHeight/2);
    glVertex2f(x + barWidth/2, y + barHeight/2);
    glVertex2f(x - barWidth/2, y + barHeight/2);
    glEnd();
    
    // Draw health (green)
    if (healthRatio > 0) {
        glColor3f(0.0f, 0.8f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(x - barWidth/2, y - barHeight/2);
        glVertex2f(x - barWidth/2 + barWidth * healthRatio, y - barHeight/2);
        glVertex2f(x - barWidth/2 + barWidth * healthRatio, y + barHeight/2);
        glVertex2f(x - barWidth/2, y + barHeight/2);
        glEnd();
    }
    
    // Draw border
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x - barWidth/2, y - barHeight/2);
    glVertex2f(x + barWidth/2, y - barHeight/2);
    glVertex2f(x + barWidth/2, y + barHeight/2);
    glVertex2f(x - barWidth/2, y + barHeight/2);
    glEnd();
    
    glColor3f(1.0f, 1.0f, 1.0f);  // Reset color
}

void UI::drawHeart(float x, float y, bool filled, float size) {
    glDisable(GL_TEXTURE_2D);
    
    if (filled) {
        glColor3f(1.0f, 0.0f, 0.0f);  // Red for filled heart
    } else {
        glColor3f(0.5f, 0.0f, 0.0f);  // Dark red for empty heart
    }
    
    // Draw heart shape using triangles
    float halfSize = size / 2.0f;
    
    // Left curve
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + halfSize * 0.5f, y + halfSize * 0.3f);  // Center of left curve
    for (int i = 0; i <= 8; i++) {
        float angle = M_PI * i / 8.0f;
        float px = x + halfSize * 0.5f + halfSize * 0.3f * cos(angle);
        float py = y + halfSize * 0.3f + halfSize * 0.3f * sin(angle);
        glVertex2f(px, py);
    }
    glEnd();
    
    // Right curve
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + halfSize * 1.5f, y + halfSize * 0.3f);  // Center of right curve
    for (int i = 0; i <= 8; i++) {
        float angle = M_PI * i / 8.0f;
        float px = x + halfSize * 1.5f + halfSize * 0.3f * cos(angle);
        float py = y + halfSize * 0.3f + halfSize * 0.3f * sin(angle);
        glVertex2f(px, py);
    }
    glEnd();
    
    // Bottom point
    glBegin(GL_TRIANGLES);
    glVertex2f(x + halfSize * 0.5f, y + halfSize * 0.3f);
    glVertex2f(x + halfSize * 1.5f, y + halfSize * 0.3f);
    glVertex2f(x + halfSize, y + halfSize * 1.2f);
    glEnd();
    
    // Draw outline
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    // Left curve outline
    for (int i = 0; i <= 8; i++) {
        float angle = M_PI * i / 8.0f;
        float px = x + halfSize * 0.5f + halfSize * 0.3f * cos(angle);
        float py = y + halfSize * 0.3f + halfSize * 0.3f * sin(angle);
        glVertex2f(px, py);
    }
    // Right curve outline
    for (int i = 8; i >= 0; i--) {
        float angle = M_PI * i / 8.0f;
        float px = x + halfSize * 1.5f + halfSize * 0.3f * cos(angle);
        float py = y + halfSize * 0.3f + halfSize * 0.3f * sin(angle);
        glVertex2f(px, py);
    }
    // Bottom point
    glVertex2f(x + halfSize, y + halfSize * 1.2f);
    glEnd();
    
    glColor3f(1.0f, 1.0f, 1.0f);  // Reset color
    glEnable(GL_TEXTURE_2D);
}

void UI::drawPixelText(const std::string&, float, float, float, float, float, float) {
    // No-op: pixel text fallback removed
}

void UI::drawMenuButton(const std::string& text, float x, float y, float width, float height, bool isHovered, bool isSelected) {
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw glow effect if selected
    if (isSelected) {
        // Outer glow (larger, more transparent)
        glColor4f(205.0f/255.0f, 133.0f/255.0f, 63.0f/255.0f, 0.3f); // RGB(205, 133, 63) with 30% alpha
        glBegin(GL_QUADS);
        glVertex2f(x - 8, y - 8);
        glVertex2f(x + width + 8, y - 8);
        glVertex2f(x + width + 8, y + height + 8);
        glVertex2f(x - 8, y + height + 8);
        glEnd();
        
        // Inner glow (smaller, more opaque)
        glColor4f(205.0f/255.0f, 133.0f/255.0f, 63.0f/255.0f, 0.6f); // RGB(205, 133, 63) with 60% alpha
        glBegin(GL_QUADS);
        glVertex2f(x - 4, y - 4);
        glVertex2f(x + width + 4, y - 4);
        glVertex2f(x + width + 4, y + height + 4);
        glVertex2f(x - 4, y + height + 4);
        glEnd();
    }
    
    // Button background - semi-transparent black
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f); // Black background with 70% alpha
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
    
    // Draw button border in RGB(205, 133, 63) with transparency
    glColor4f(205.0f/255.0f, 133.0f/255.0f, 63.0f/255.0f, 0.8f); // RGB(205, 133, 63) with 80% alpha
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Reset color
    glEnable(GL_TEXTURE_2D);
    
    // Draw text using FreeType in RGB(205, 133, 63)
    float textX = x + width / 2.0f;
    float textY = y + height / 2.0f - 10.0f;  // Moved up by 3 pixels (from -5 to -8)
    drawCenteredText(text, textX, textY, 0.8f, 205.0f/255.0f, 133.0f/255.0f, 63.0f/255.0f);
}

void UI::drawMainMenu(int windowWidth, int windowHeight, int selectedOption, bool hasSaveFile) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Draw background texture if available, otherwise fall back to black
    if (titleScreenTextureID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, titleScreenTextureID);
        glColor3f(1.0f, 1.0f, 1.0f);  // White color to show texture as-is
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 0);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(windowWidth, 0);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(windowWidth, windowHeight);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0, windowHeight);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        // Fallback to black background if texture not loaded
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.0f, 0.0f, 0.0f);  // Black background
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(windowWidth, 0);
        glVertex2f(windowWidth, windowHeight);
        glVertex2f(0, windowHeight);
        glEnd();
    }
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    
    // Draw buttons (slightly offset to the left)
    float buttonWidth = 260.0f;  // Increased from 200 to 250
    float buttonHeight = 60.0f;
    float buttonX = windowWidth / 2.0f - buttonWidth / 2.0f - 45.0f;  // Move 45 pixels to the left
    
    if (hasSaveFile) {
        // Menu with save file: Start Game, Load Game, Settings, Exit Game
        float startButtonY = windowHeight * 0.55f;
        float loadButtonY = windowHeight * 0.45f;
        float settingsButtonY = windowHeight * 0.35f;
        float exitButtonY = windowHeight * 0.25f;
        
        drawMenuButton("Start Game", buttonX, startButtonY, buttonWidth, buttonHeight, false, selectedOption == 0);
        drawMenuButton("Load Game", buttonX, loadButtonY, buttonWidth, buttonHeight, false, selectedOption == 1);
        drawMenuButton("Settings", buttonX, settingsButtonY, buttonWidth, buttonHeight, false, selectedOption == 2);
        drawMenuButton("Exit Game", buttonX, exitButtonY, buttonWidth, buttonHeight, false, selectedOption == 3);
    } else {
        // Menu without save file: Start Game, Settings, Exit Game
        float startButtonY = windowHeight * 0.5f;
        float settingsButtonY = windowHeight * 0.4f;
        float exitButtonY = windowHeight * 0.3f;
        
        drawMenuButton("Start Game", buttonX, startButtonY, buttonWidth, buttonHeight, false, selectedOption == 0);
        drawMenuButton("Settings", buttonX, settingsButtonY, buttonWidth, buttonHeight, false, selectedOption == 1);
        drawMenuButton("Exit Game", buttonX, exitButtonY, buttonWidth, buttonHeight, false, selectedOption == 2);
    }
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

bool UI::isMouseOverButton(float mouseX, float mouseY, float buttonX, float buttonY, float buttonWidth, float buttonHeight) {
    return mouseX >= buttonX && mouseX <= buttonX + buttonWidth &&
           mouseY >= buttonY && mouseY <= buttonY + buttonHeight;
}

void UI::drawDeathScreen(int windowWidth, int windowHeight, bool respawnButtonHovered, bool exitButtonHovered, int selectedButton) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Draw background texture if available, otherwise fall back to black
    if (deathScreenTextureID != 0) {
        // First fill the entire screen with black
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.0f, 0.0f, 0.0f);  // Black background
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(windowWidth, 0);
        glVertex2f(windowWidth, windowHeight);
        glVertex2f(0, windowHeight);
        glEnd();
        
        // Then draw the texture on the right side (offset to the right)
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, deathScreenTextureID);
        glColor3f(1.0f, 1.0f, 1.0f);  // White color to show texture as-is
        
        float textureOffset = 100.0f;  // Move texture 100 pixels to the right
        float textureWidth = windowWidth - textureOffset;  // Adjust texture width
        
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(textureOffset, 0);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(windowWidth, 0);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(windowWidth, windowHeight);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(textureOffset, windowHeight);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        // Fallback to black background if texture not loaded
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.0f, 0.0f, 0.0f);  // Black background
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(windowWidth, 0);
        glVertex2f(windowWidth, windowHeight);
        glVertex2f(0, windowHeight);
        glEnd();
    }
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    
    // Draw respawn and exit buttons (same position as main menu)
    float buttonWidth = 260.0f;  // Increased from 200 to 250 to match main menu
    float buttonHeight = 60.0f;
    float buttonX = windowWidth / 2.0f - buttonWidth / 2.0f - 45.0f;  // Same offset as main menu
    float respawnButtonY = windowHeight * 0.5f;
    float exitButtonY = windowHeight * 0.35f;
    
    drawMenuButton("RESPAWN", buttonX, respawnButtonY, buttonWidth, buttonHeight, respawnButtonHovered, selectedButton == 0);
    drawMenuButton("EXIT GAME", buttonX, exitButtonY, buttonWidth, buttonHeight, exitButtonHovered, selectedButton == 1);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UI::drawPauseScreen(int windowWidth, int windowHeight, int selectedButton) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Draw dark overlay (semi-transparent black)
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f); // Black with 70% alpha for dark overlay
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(0, windowHeight);
    glEnd();
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Reset color
    glEnable(GL_TEXTURE_2D);
    
    // Draw pause menu buttons
    float buttonWidth = 260.0f;
    float buttonHeight = 60.0f;
    float buttonX = windowWidth / 2.0f - buttonWidth / 2.0f - 45.0f; // Same offset as other menus
    
    // Resume button (top)
    float resumeButtonY = windowHeight * 0.65f;
    drawMenuButton("Resume", buttonX, resumeButtonY, buttonWidth, buttonHeight, false, selectedButton == 0);
    
    // Save Game button
    float saveButtonY = windowHeight * 0.55f;
    drawMenuButton("Save Game", buttonX, saveButtonY, buttonWidth, buttonHeight, false, selectedButton == 1);
    
    // Settings button
    float settingsButtonY = windowHeight * 0.45f;
    drawMenuButton("Settings", buttonX, settingsButtonY, buttonWidth, buttonHeight, false, selectedButton == 2);
    
    // Back to Menu button (middle)
    float menuButtonY = windowHeight * 0.35f;
    drawMenuButton("Back to Menu", buttonX, menuButtonY, buttonWidth, buttonHeight, false, selectedButton == 3);
    
    // Exit Game button (bottom)
    float exitButtonY = windowHeight * 0.25f;
    drawMenuButton("Exit Game", buttonX, exitButtonY, buttonWidth, buttonHeight, false, selectedButton == 4);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UI::drawSaveSlotMenu(int windowWidth, int windowHeight, int selectedSlot, const std::vector<std::string>& saveSlotInfo) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Draw dark overlay
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(0, windowHeight);
    glEnd();
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    
    // Draw title
    drawCenteredText("Select Save Slot", windowWidth / 2.0f, windowHeight * 0.8f, 1.5f);
    
    // Draw save slots using same layout as main menu
    float buttonWidth = 350.0f;  // Wider for save slot text
    float buttonHeight = 60.0f;  // Same as main menu
    float buttonX = windowWidth / 2.0f - buttonWidth / 2.0f - 45.0f;  // Same offset as main menu
    
    // Position buttons like main menu: 0.6f, 0.5f, 0.4f, 0.3f
    float slot1Y = windowHeight * 0.6f;
    float slot2Y = windowHeight * 0.5f;
    float slot3Y = windowHeight * 0.4f;
    float backButtonY = windowHeight * 0.3f;
    
    // Save Slot 1
    std::string buttonText1 = "Save Slot 1";
    if (0 < saveSlotInfo.size() && saveSlotInfo[0] != "Empty") {
        std::string shortTime = saveSlotInfo[0].substr(0, 10);
        buttonText1 += " - " + shortTime;
    } else {
        buttonText1 += " - Empty";
    }
    drawMenuButton(buttonText1, buttonX, slot1Y, buttonWidth, buttonHeight, false, selectedSlot == 0);
    
    // Save Slot 2
    std::string buttonText2 = "Save Slot 2";
    if (1 < saveSlotInfo.size() && saveSlotInfo[1] != "Empty") {
        std::string shortTime = saveSlotInfo[1].substr(0, 10);
        buttonText2 += " - " + shortTime;
    } else {
        buttonText2 += " - Empty";
    }
    drawMenuButton(buttonText2, buttonX, slot2Y, buttonWidth, buttonHeight, false, selectedSlot == 1);
    
    // Save Slot 3
    std::string buttonText3 = "Save Slot 3";
    if (2 < saveSlotInfo.size() && saveSlotInfo[2] != "Empty") {
        std::string shortTime = saveSlotInfo[2].substr(0, 10);
        buttonText3 += " - " + shortTime;
    } else {
        buttonText3 += " - Empty";
    }
    drawMenuButton(buttonText3, buttonX, slot3Y, buttonWidth, buttonHeight, false, selectedSlot == 2);
    
    // Back button
    drawMenuButton("Back", buttonX, backButtonY, buttonWidth, buttonHeight, false, selectedSlot == 3);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UI::drawLoadSlotMenu(int windowWidth, int windowHeight, int selectedSlot, const std::vector<std::string>& saveSlotInfo) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Draw dark overlay
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(0, windowHeight);
    glEnd();
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    
    // Draw title
    drawCenteredText("Select Load Slot", windowWidth / 2.0f, windowHeight * 0.8f, 1.5f);
    
    // Draw save slots using same layout as main menu
    float buttonWidth = 350.0f;  // Wider for save slot text
    float buttonHeight = 60.0f;  // Same as main menu
    float buttonX = windowWidth / 2.0f - buttonWidth / 2.0f - 45.0f;  // Same offset as main menu
    
    // Position buttons like main menu: 0.6f, 0.5f, 0.4f, 0.3f
    float slot1Y = windowHeight * 0.6f;
    float slot2Y = windowHeight * 0.5f;
    float slot3Y = windowHeight * 0.4f;
    float backButtonY = windowHeight * 0.3f;
    
    // Save Slot 1
    std::string buttonText1 = "Save Slot 1";
    if (0 < saveSlotInfo.size()) {
        if (saveSlotInfo[0] == "Empty") {
            buttonText1 += " - Empty";
        } else {
            std::string shortTime = saveSlotInfo[0].substr(0, 10);
            buttonText1 += " - " + shortTime;
        }
    } else {
        buttonText1 += " - Empty";
    }
    drawMenuButton(buttonText1, buttonX, slot1Y, buttonWidth, buttonHeight, false, selectedSlot == 0);
    
    // Save Slot 2
    std::string buttonText2 = "Save Slot 2";
    if (1 < saveSlotInfo.size()) {
        if (saveSlotInfo[1] == "Empty") {
            buttonText2 += " - Empty";
        } else {
            std::string shortTime = saveSlotInfo[1].substr(0, 10);
            buttonText2 += " - " + shortTime;
        }
    } else {
        buttonText2 += " - Empty";
    }
    drawMenuButton(buttonText2, buttonX, slot2Y, buttonWidth, buttonHeight, false, selectedSlot == 1);
    
    // Save Slot 3
    std::string buttonText3 = "Save Slot 3";
    if (2 < saveSlotInfo.size()) {
        if (saveSlotInfo[2] == "Empty") {
            buttonText3 += " - Empty";
        } else {
            std::string shortTime = saveSlotInfo[2].substr(0, 10);
            buttonText3 += " - " + shortTime;
        }
    } else {
        buttonText3 += " - Empty";
    }
    drawMenuButton(buttonText3, buttonX, slot3Y, buttonWidth, buttonHeight, false, selectedSlot == 2);
    
    // Back button
    drawMenuButton("Back", buttonX, backButtonY, buttonWidth, buttonHeight, false, selectedSlot == 3);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UI::drawSettingsMenu(int windowWidth, int windowHeight, int selectedOption, float masterVolume, float musicVolume, float sfxVolume) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Draw background texture if available, otherwise fall back to black
    if (titleScreenTextureID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, titleScreenTextureID);
        glColor3f(1.0f, 1.0f, 1.0f);  // White color to show texture as-is
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 0);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(windowWidth, 0);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(windowWidth, windowHeight);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0, windowHeight);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        // Fallback to black background if texture not loaded
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.0f, 0.0f, 0.0f);  // Black background
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(windowWidth, 0);
        glVertex2f(windowWidth, windowHeight);
        glVertex2f(0, windowHeight);
        glEnd();
    }
    
    // Draw semi-transparent dark overlay for better text readability
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f); // Black with 50% alpha for subtle overlay
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(0, windowHeight);
    glEnd();
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Reset color
    glEnable(GL_TEXTURE_2D);
    
    // Draw settings title
    float titleY = windowHeight * 0.85f;
    textRenderer->renderText("Settings", windowWidth / 2.0f - 80, titleY, 1.5f, 1.0f, 1.0f, 1.0f);
    
    // Draw settings options
    float buttonWidth = 300.0f;
    float buttonHeight = 50.0f;
    float buttonX = windowWidth / 2.0f - buttonWidth / 2.0f;
    
    // Master Volume button
    float masterVolumeY = windowHeight * 0.6f;
    std::string masterText = "Master Volume: " + std::to_string((int)(masterVolume * 100)) + "%";
    drawMenuButton(masterText, buttonX, masterVolumeY, buttonWidth, buttonHeight, false, selectedOption == 0);
    
    // Volume bar for Master (drawn separately)
    float barWidth = 250.0f;
    float barHeight = 25.0f;
    float barX = windowWidth / 2.0f - barWidth / 2.0f;
    float masterBarY = masterVolumeY - 50.0f;
    
    // Background bar
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(barX, masterBarY);
    glVertex2f(barX + barWidth, masterBarY);
    glVertex2f(barX + barWidth, masterBarY + barHeight);
    glVertex2f(barX, masterBarY + barHeight);
    glEnd();
    
    // Volume bar
    glColor3f(selectedOption == 0 ? 1.0f : 0.7f, selectedOption == 0 ? 1.0f : 0.7f, selectedOption == 0 ? 1.0f : 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(barX, masterBarY);
    glVertex2f(barX + barWidth * masterVolume, masterBarY);
    glVertex2f(barX + barWidth * masterVolume, masterBarY + barHeight);
    glVertex2f(barX, masterBarY + barHeight);
    glEnd();
    
    // Back button
    float backButtonY = windowHeight * 0.3f;
    drawMenuButton("Back", buttonX, backButtonY, buttonWidth, buttonHeight, false, selectedOption == 1);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
} 