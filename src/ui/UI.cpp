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
    
    initialized = false;
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
    spdlog::info("Drawing health bar: HP {}/{}", currentHealth, maxHealth);
    
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

void UI::drawMainMenu(int windowWidth, int windowHeight, int selectedOption) {
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
    
    // Start Game button (moved lower)
    float startButtonY = windowHeight * 0.5f;
    drawMenuButton("Start Game", buttonX, startButtonY, buttonWidth, buttonHeight, false, selectedOption == 0);
    
    // Exit Game button (moved lower)
    float exitButtonY = windowHeight * 0.35f;
    drawMenuButton("Exit Game", buttonX, exitButtonY, buttonWidth, buttonHeight, false, selectedOption == 1);
    
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