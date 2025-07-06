#include "projectile/Projectile.h"
#include "map/TileMap.h"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <cmath>
#include <stb_image.h>

// Static member initialization
unsigned int Projectile::textureID = 0;
int Projectile::textureWidth = 0;
int Projectile::textureHeight = 0;
int Projectile::spriteWidth = 16;   // 16x16 pixels per sprite
int Projectile::spriteHeight = 16;  // 16x16 pixels per sprite
bool Projectile::textureLoaded = false;
unsigned int Projectile::eyeTextureID = 0;
int Projectile::eyeTextureWidth = 0;
int Projectile::eyeTextureHeight = 0;
bool Projectile::eyeTextureLoaded = false;
unsigned int Projectile::shroomTextureID = 0;
int Projectile::shroomTextureWidth = 0;
int Projectile::shroomTextureHeight = 0;
bool Projectile::shroomTextureLoaded = false;

Projectile::Projectile(float x, float y, float dx, float dy, ProjectileType type)
    : x(x), y(y), dx(dx), dy(dy), type(type),
      animationTimer(0.0f), frameDuration(0.1f), currentFrame(0), totalFrames(5) {
    
    // Normalize direction vector
    float length = std::sqrt(dx * dx + dy * dy);
    if (length > 0) {
        this->dx = dx / length;
        this->dy = dy / length;
    }
    
    // Set color based on type (fallback for enemy projectiles)
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

void Projectile::loadProjectileTexture(const std::string& filePath) {
    if (textureLoaded) {
        spdlog::warn("Projectile texture already loaded");
        return;
    }
    
    int width, height, channels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        spdlog::error("Failed to load projectile texture: {}", filePath);
        return;
    }
    
    spdlog::info("Loaded projectile texture: {} ({}x{})", filePath, width, height);
    spdlog::info("Projectile texture dimensions: {}x{} pixels, {} sprites per row, {} rows total", 
                 width, height, width / spriteWidth, height / spriteHeight);
    
    textureWidth = width;
    textureHeight = height;
    
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
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
    
    textureLoaded = true;
    spdlog::debug("Projectile texture loaded successfully with ID: {}", textureID);
}

void Projectile::cleanupProjectileTexture() {
    if (textureLoaded && textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
        textureLoaded = false;
        spdlog::debug("Projectile texture cleaned up");
    }
}

// Add new texture loading functions for eye and shroom projectiles
void Projectile::loadEyeProjectileTexture(const std::string& filePath) {
    if (eyeTextureLoaded) {
        spdlog::warn("Eye projectile texture already loaded");
        return;
    }
    int width, height, channels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        spdlog::error("Failed to load eye projectile texture: {}", filePath);
        return;
    }
    eyeTextureWidth = width;
    eyeTextureHeight = height;
    glGenTextures(1, &eyeTextureID);
    glBindTexture(GL_TEXTURE_2D, eyeTextureID);
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
    eyeTextureLoaded = true;
    spdlog::info("Loaded eye projectile texture: {} ({}x{})", filePath, width, height);
}

void Projectile::loadShroomProjectileTexture(const std::string& filePath) {
    if (shroomTextureLoaded) {
        spdlog::warn("Shroom projectile texture already loaded");
        return;
    }
    int width, height, channels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        spdlog::error("Failed to load shroom projectile texture: {}", filePath);
        return;
    }
    shroomTextureWidth = width;
    shroomTextureHeight = height;
    glGenTextures(1, &shroomTextureID);
    glBindTexture(GL_TEXTURE_2D, shroomTextureID);
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
    shroomTextureLoaded = true;
    spdlog::info("Loaded shroom projectile texture: {} ({}x{})", filePath, width, height);
}

void Projectile::loadAllProjectileTextures() {
    loadProjectileTexture("assets/graphic/projectiles/green_projectiles.png");
    loadEyeProjectileTexture("assets/graphic/projectiles/purple_projectiles.png");
    loadShroomProjectileTexture("assets/graphic/projectiles/pink_projectiles.png");
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
    
    // Update animation
    animationTimer += deltaTime;
    if (animationTimer >= frameDuration) {
        animationTimer -= frameDuration;
        currentFrame = (currentFrame + 1) % totalFrames;
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
    
    // Player projectile
    if (type == ProjectileType::PlayerBullet && textureLoaded) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
        int row = PLAYER_ROW;
        int col = currentFrame % 5;  // 5 sprites per row in green_projectiles.png
        float u1 = static_cast<float>(col * spriteWidth) / textureWidth;
        float v1 = static_cast<float>(row * spriteHeight) / textureHeight;
        float u2 = static_cast<float>((col + 1) * spriteWidth) / textureWidth;
        float v2 = static_cast<float>((row + 1) * spriteHeight) / textureHeight;
        bool facingRight = dx > 0;
        if (!facingRight) std::swap(u1, u2);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(u1, v2); glVertex2f(x - spriteWidth/2, y - spriteHeight/2);
        glTexCoord2f(u2, v2); glVertex2f(x + spriteWidth/2, y - spriteHeight/2);
        glTexCoord2f(u2, v1); glVertex2f(x + spriteWidth/2, y + spriteHeight/2);
        glTexCoord2f(u1, v1); glVertex2f(x - spriteWidth/2, y + spriteHeight/2);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
        return;
    }
    // Eye enemy projectile
    if (type == ProjectileType::EnemyEyeBullet && eyeTextureLoaded) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, eyeTextureID);
        int row = EYE_ROW;
        int col = currentFrame % 5;  // 5 sprites per row in purple_projectiles.png
        float u1 = static_cast<float>(col * spriteWidth) / eyeTextureWidth;
        float v1 = static_cast<float>(row * spriteHeight) / eyeTextureHeight;
        float u2 = static_cast<float>((col + 1) * spriteWidth) / eyeTextureWidth;
        float v2 = static_cast<float>((row + 1) * spriteHeight) / eyeTextureHeight;
        bool facingRight = dx > 0;
        if (!facingRight) std::swap(u1, u2);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(u1, v2); glVertex2f(x - spriteWidth/2, y - spriteHeight/2);
        glTexCoord2f(u2, v2); glVertex2f(x + spriteWidth/2, y - spriteHeight/2);
        glTexCoord2f(u2, v1); glVertex2f(x + spriteWidth/2, y + spriteHeight/2);
        glTexCoord2f(u1, v1); glVertex2f(x - spriteWidth/2, y + spriteHeight/2);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
        return;
    }
    // Shroom enemy projectile
    if (type == ProjectileType::EnemyShroomBullet && shroomTextureLoaded) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, shroomTextureID);
        int row = SHROOM_ROW;
        int col = currentFrame % 5;  // 5 sprites per row in pink_projectiles.png
        float u1 = static_cast<float>(col * spriteWidth) / shroomTextureWidth;
        float v1 = static_cast<float>(row * spriteHeight) / shroomTextureHeight;
        float u2 = static_cast<float>((col + 1) * spriteWidth) / shroomTextureWidth;
        float v2 = static_cast<float>((row + 1) * spriteHeight) / shroomTextureHeight;
        bool facingRight = dx > 0;
        if (!facingRight) std::swap(u1, u2);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(u1, v2); glVertex2f(x - spriteWidth/2, y - spriteHeight/2);
        glTexCoord2f(u2, v2); glVertex2f(x + spriteWidth/2, y - spriteHeight/2);
        glTexCoord2f(u2, v1); glVertex2f(x + spriteWidth/2, y + spriteHeight/2);
        glTexCoord2f(u1, v1); glVertex2f(x - spriteWidth/2, y + spriteHeight/2);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
        return;
    }
    // Fallback: circle for other enemy projectiles
    glDisable(GL_TEXTURE_2D);
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 16; i++) {
        float angle = 2.0f * M_PI * i / 16.0f;
        float px = x + radius * std::cos(angle);
        float py = y + radius * std::sin(angle);
        glVertex2f(px, py);
    }
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 16; i++) {
        float angle = 2.0f * M_PI * i / 16.0f;
        float px = x + radius * std::cos(angle);
        float py = y + radius * std::sin(angle);
        glVertex2f(px, py);
    }
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
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