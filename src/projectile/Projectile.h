#pragma once
#include <string>

enum class ProjectileType {
    PlayerBullet,
    EnemyBullet,
    EnemyEyeBullet,    // For FlyingEye
    EnemyShroomBullet  // For Shroom
};

class Projectile {
public:
    Projectile(float x, float y, float dx, float dy, ProjectileType type);
    ~Projectile();

    void update(float deltaTime);
    void draw() const;
    bool isActive() const { return active; }
    void setActive(bool isActive) { active = isActive; }
    
    // Position getters
    float getX() const { return x; }
    float getY() const { return y; }
    float getRadius() const { return radius; }
    
    // Collision detection
    bool checkCollision(float targetX, float targetY, float targetRadius) const;
    bool checkWallCollision(const class Tilemap& tilemap) const;
    
    // Projectile properties
    ProjectileType getType() const { return type; }
    float getSpeed() const { return speed; }

    // Static texture loading
    static void loadProjectileTexture(const std::string& filePath);
    static void cleanupProjectileTexture();
    static void loadEyeProjectileTexture(const std::string& filePath);
    static void loadShroomProjectileTexture(const std::string& filePath);
    static void loadAllProjectileTextures();

private:
    float x, y;
    float dx, dy;  // Direction vector
    float speed = 200.0f;
    float radius = 4.0f;
    float lifetime = 5.0f;  // How long the projectile lives
    float currentLifetime = 0.0f;
    bool active = true;
    ProjectileType type;
    
    // Visual properties
    float r, g, b;  // Color (fallback for enemy projectiles)
    
    // Static texture properties
    static unsigned int textureID;
    static int textureWidth;
    static int textureHeight;
    static int spriteWidth;
    static int spriteHeight;
    static bool textureLoaded;

    // Additional textures for enemy projectiles
    static unsigned int eyeTextureID;
    static int eyeTextureWidth;
    static int eyeTextureHeight;
    static bool eyeTextureLoaded;
    static unsigned int shroomTextureID;
    static int shroomTextureWidth;
    static int shroomTextureHeight;
    static bool shroomTextureLoaded;

    // Sprite row info for each type
    static constexpr int PLAYER_ROW = 14; // 15th row (0-indexed)
    static constexpr int EYE_ROW = 11;    // 12th row (0-indexed)
    static constexpr int SHROOM_ROW = 14; // 15th row (0-indexed)
    
    // Animation properties
    float animationTimer;
    float frameDuration;
    int currentFrame;
    int totalFrames;
}; 