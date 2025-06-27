#include "UI.h"
#include <cmath>
#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Static member initialization
TextRenderer* UI::textRenderer = nullptr;
bool UI::initialized = false;

bool UI::init(const std::string& fontPath) {
    if (initialized) {
        spdlog::warn("UI already initialized!");
        return true;
    }
    
    spdlog::info("Initializing UI system with font: {}", fontPath);
    textRenderer = new TextRenderer();
    if (!textRenderer->init(fontPath)) {
        spdlog::error("Failed to initialize TextRenderer with font: {}", fontPath);
        delete textRenderer;
        textRenderer = nullptr;
        return false;
    }
    
    initialized = true;
    spdlog::info("UI system initialized successfully with textRenderer: {}", (textRenderer != nullptr));
    return true;
}

void UI::cleanup() {
    if (textRenderer) {
        textRenderer->cleanup();
        delete textRenderer;
        textRenderer = nullptr;
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
    // Button background colors for dark theme
    if (isSelected) {
        glColor3f(0.4f, 0.35f, 0.5f); // Dusty muted purple-gray for selected
    } else if (isHovered) {
        glColor3f(0.35f, 0.35f, 0.4f); // Slightly lighter gray for hover
    } else {
        glColor3f(0.25f, 0.25f, 0.3f); // Dark gray for normal
    }
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
    // Draw button border
    glColor3f(0.15f, 0.15f, 0.18f); // Even darker border
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f); // Reset color
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Draw text using FreeType (darker, muted red)
    float textX = x + width / 2.0f;
    float textY = y + height / 2.0f - 5.0f;
    drawCenteredText(text, textX, textY, 0.8f, 0.7f, 0.2f, 0.2f);
}

void UI::drawMainMenu(int windowWidth, int windowHeight, int selectedOption) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    // Draw background (black)
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.0f, 0.0f, 0.0f);  // Black background
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(0, windowHeight);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    // Draw title using FreeType (darker red)
    float titleY = windowHeight * 0.8f;
    drawCenteredText("Ortos II", windowWidth / 2.0f, titleY, 3.0f, 0.7f, 0.2f, 0.2f);  // Muted red
    // Draw buttons
    float buttonWidth = 200.0f;
    float buttonHeight = 60.0f;
    float buttonX = windowWidth / 2.0f - buttonWidth / 2.0f;
    // Start Game button
    float startButtonY = windowHeight * 0.6f;
    drawMenuButton("Start Game", buttonX, startButtonY, buttonWidth, buttonHeight, false, selectedOption == 0);
    // Exit Game button
    float exitButtonY = windowHeight * 0.4f;
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
    // Draw black background
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.0f, 0.0f, 0.0f);  // Black background
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(0, windowHeight);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    // Draw "YOU DIED" text using FreeType (darker red)
    float titleY = windowHeight * 0.8f;
    drawCenteredText("YOU DIED", windowWidth / 2.0f, titleY, 3.0f, 0.7f, 0.2f, 0.2f);  // Muted red
    // Draw respawn and exit buttons
    float buttonWidth = 200.0f;
    float buttonHeight = 60.0f;
    float buttonX = windowWidth / 2.0f - buttonWidth / 2.0f;
    float respawnButtonY = windowHeight * 0.5f;
    float exitButtonY = windowHeight * 0.35f;
    drawMenuButton("RESPAWN", buttonX, respawnButtonY, buttonWidth, buttonHeight, respawnButtonHovered, selectedButton == 0);
    drawMenuButton("EXIT GAME", buttonX, exitButtonY, buttonWidth, buttonHeight, exitButtonHovered, selectedButton == 1);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
} 