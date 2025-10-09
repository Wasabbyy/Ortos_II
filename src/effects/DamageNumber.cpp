#include "effects/DamageNumber.h"
#include "ui/UI.h"
#include "ui/TextRenderer.h"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <string>

DamageNumber::DamageNumber(float x, float y, int damage, bool isPlayerDamage)
    : x(x), y(y), damage(damage), isPlayerDamage(isPlayerDamage),
      active(true), finished(false),
      lifetime(0.0f), maxLifetime(1.5f), floatSpeed(30.0f), alpha(1.0f) {
    spdlog::debug("DamageNumber created at ({}, {}) with damage: {}", x, y, damage);
}

DamageNumber::~DamageNumber() {
}

void DamageNumber::update(float deltaTime) {
    if (!active) return;
    
    lifetime += deltaTime;
    
    // Float upwards
    y -= floatSpeed * deltaTime;
    
    // Fade out over time
    alpha = 1.0f - (lifetime / maxLifetime);
    
    // Deactivate when lifetime is exceeded
    if (lifetime >= maxLifetime) {
        active = false;
        finished = true;
    }
}

void DamageNumber::draw() const {
    if (!active || alpha <= 0.0f) return;
    
    // Set color based on damage type
    float r, g, b;
    if (isPlayerDamage) {
        // Red for player damage
        r = 1.0f;
        g = 0.2f;
        b = 0.2f;
    } else {
        // Yellow/Gold for enemy damage
        r = 1.0f;
        g = 0.85f;
        b = 0.0f;
    }
    
    std::string damageText = std::to_string(damage);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    
    // Draw black outline/shadow for visibility (4 directions only for performance)
    drawTextInWorldSpace(damageText, x - 1, y, 1.0f, 0.0f, 0.0f, 0.0f, alpha * 0.9f);
    drawTextInWorldSpace(damageText, x + 1, y, 1.0f, 0.0f, 0.0f, 0.0f, alpha * 0.9f);
    drawTextInWorldSpace(damageText, x, y - 1, 1.0f, 0.0f, 0.0f, 0.0f, alpha * 0.9f);
    drawTextInWorldSpace(damageText, x, y + 1, 1.0f, 0.0f, 0.0f, 0.0f, alpha * 0.9f);
    
    // Draw the main colored rectangles
    drawTextInWorldSpace(damageText, x, y, 1.0f, r, g, b, alpha);
    
    // Reset states
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
}

void DamageNumber::drawTextInWorldSpace(const std::string& text, float worldX, float worldY, 
                                         float scale, float r, float g, float b, float a) const {
    // Simple pixel-art style number renderer in world coordinates
    
    glDisable(GL_TEXTURE_2D);
    glColor4f(r, g, b, a);
    
    float pixelSize = 1.2f;  // Size of each pixel in the digit
    float charWidth = 5.0f * pixelSize;  // 5 pixels wide
    float charHeight = 7.0f * pixelSize; // 7 pixels tall
    float spacing = pixelSize * 2.0f;
    float totalWidth = text.length() * (charWidth + spacing);
    float startX = worldX - totalWidth / 2.0f;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        if (c < '0' || c > '9') continue; // Only render digits
        
        float charX = startX + i * (charWidth + spacing);
        drawDigitShape(c - '0', charX, worldY, pixelSize);
    }
    
    glEnable(GL_TEXTURE_2D);
}

void DamageNumber::drawDigitShape(int digit, float x, float y, float pixelSize) const {
    // 5x7 pixel art font for digits 0-9
    // Each digit is represented as a 5x7 grid where 1 = pixel on, 0 = pixel off
    
    static const bool digitPatterns[10][7][5] = {
        // 0
        {{0,1,1,1,0},
         {1,0,0,0,1},
         {1,0,0,0,1},
         {1,0,0,0,1},
         {1,0,0,0,1},
         {1,0,0,0,1},
         {0,1,1,1,0}},
        // 1
        {{0,0,1,0,0},
         {0,1,1,0,0},
         {0,0,1,0,0},
         {0,0,1,0,0},
         {0,0,1,0,0},
         {0,0,1,0,0},
         {0,1,1,1,0}},
        // 2
        {{0,1,1,1,0},
         {1,0,0,0,1},
         {0,0,0,0,1},
         {0,0,0,1,0},
         {0,0,1,0,0},
         {0,1,0,0,0},
         {1,1,1,1,1}},
        // 3
        {{0,1,1,1,0},
         {1,0,0,0,1},
         {0,0,0,0,1},
         {0,0,1,1,0},
         {0,0,0,0,1},
         {1,0,0,0,1},
         {0,1,1,1,0}},
        // 4
        {{0,0,0,1,0},
         {0,0,1,1,0},
         {0,1,0,1,0},
         {1,0,0,1,0},
         {1,1,1,1,1},
         {0,0,0,1,0},
         {0,0,0,1,0}},
        // 5
        {{1,1,1,1,1},
         {1,0,0,0,0},
         {1,1,1,1,0},
         {0,0,0,0,1},
         {0,0,0,0,1},
         {1,0,0,0,1},
         {0,1,1,1,0}},
        // 6
        {{0,0,1,1,0},
         {0,1,0,0,0},
         {1,0,0,0,0},
         {1,1,1,1,0},
         {1,0,0,0,1},
         {1,0,0,0,1},
         {0,1,1,1,0}},
        // 7
        {{1,1,1,1,1},
         {0,0,0,0,1},
         {0,0,0,1,0},
         {0,0,1,0,0},
         {0,1,0,0,0},
         {0,1,0,0,0},
         {0,1,0,0,0}},
        // 8
        {{0,1,1,1,0},
         {1,0,0,0,1},
         {1,0,0,0,1},
         {0,1,1,1,0},
         {1,0,0,0,1},
         {1,0,0,0,1},
         {0,1,1,1,0}},
        // 9
        {{0,1,1,1,0},
         {1,0,0,0,1},
         {1,0,0,0,1},
         {0,1,1,1,1},
         {0,0,0,0,1},
         {0,0,0,1,0},
         {0,1,1,0,0}}
    };
    
    if (digit < 0 || digit > 9) return;
    
    // Draw each pixel of the digit
    glBegin(GL_QUADS);
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (digitPatterns[digit][row][col]) {
                float px = x + col * pixelSize;
                float py = y + row * pixelSize;
                
                glVertex2f(px, py);
                glVertex2f(px + pixelSize, py);
                glVertex2f(px + pixelSize, py + pixelSize);
                glVertex2f(px, py + pixelSize);
            }
        }
    }
    glEnd();
}

