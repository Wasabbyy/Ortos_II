#pragma once
#include <string>

enum class ProjectileType {
    PlayerBullet,
    EnemyBullet
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
    float r, g, b;  // Color
}; 