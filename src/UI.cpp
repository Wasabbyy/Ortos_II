#include "UI.h"
#include <cmath>
#include <spdlog/spdlog.h>

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