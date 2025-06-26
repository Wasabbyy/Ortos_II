#include "Projectile.h"
#include "TileMap.h"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <cmath>

Projectile::Projectile(float x, float y, float dx, float dy, ProjectileType type)
    : x(x), y(y), dx(dx), dy(dy), type(type) {
    
    // Normalize direction vector
    float length = std::sqrt(dx * dx + dy * dy);
    if (length > 0) {
        this->dx = dx / length;
        this->dy = dy / length;
    }
    
    // Set color based on type
    if (type == ProjectileType::PlayerBullet) {
        r = 0.0f; g = 1.0f; b = 0.0f;  // Green for player
    } else {
        r = 1.0f; g = 0.0f; b = 0.0f;  // Red for enemy
    }
    
    spdlog::debug("Projectile created at ({}, {}) with direction ({}, {})", x, y, this->dx, this->dy);
}

Projectile::~Projectile() {
    // Cleanup if needed
}

void Projectile::update(float deltaTime) {
    if (!active) return;
    
    // Update lifetime
    currentLifetime += deltaTime;
    if (currentLifetime >= lifetime) {
        active = false;
        spdlog::debug("Projectile expired after {} seconds", currentLifetime);
        return;
    }
    
    // Move projectile
    float oldX = x;
    float oldY = y;
    x += dx * speed * deltaTime;
    y += dy * speed * deltaTime;
    
    spdlog::debug("Projectile moved from ({}, {}) to ({}, {})", oldX, oldY, x, y);
}

void Projectile::draw() const {
    if (!active) return;
    
    // Draw projectile as a colored circle
    glDisable(GL_TEXTURE_2D);
    glColor3f(r, g, b);
    
    // Draw filled circle
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);  // Center
    for (int i = 0; i <= 16; i++) {
        float angle = 2.0f * M_PI * i / 16.0f;
        float px = x + radius * std::cos(angle);
        float py = y + radius * std::sin(angle);
        glVertex2f(px, py);
    }
    glEnd();
    
    // Draw outline
    glColor3f(1.0f, 1.0f, 1.0f);  // White outline
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 16; i++) {
        float angle = 2.0f * M_PI * i / 16.0f;
        float px = x + radius * std::cos(angle);
        float py = y + radius * std::sin(angle);
        glVertex2f(px, py);
    }
    glEnd();
    
    glColor3f(1.0f, 1.0f, 1.0f);  // Reset color
    glEnable(GL_TEXTURE_2D);
}

bool Projectile::checkCollision(float targetX, float targetY, float targetRadius) const {
    if (!active) return false;
    
    float dx = x - targetX;
    float dy = y - targetY;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    return distance <= (radius + targetRadius);
}

bool Projectile::checkWallCollision(const Tilemap& tilemap) const {
    if (!active) return false;
    
    // Convert projectile position to tile coordinates
    int tileX = static_cast<int>(x / tilemap.getTileWidth());
    int tileY = static_cast<int>(y / tilemap.getTileHeight());
    
    // Check if the tile at projectile position is solid
    if (tilemap.isTileSolid(tileX, tileY)) {
        spdlog::debug("Projectile hit wall at tile ({}, {})", tileX, tileY);
        return true;
    }
    
    return false;
} 