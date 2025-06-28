#pragma once
#include <string>
#include <vector>
#include <random>

enum class EnemyType {
    Skeleton,
    Zombie,
    Ghost
};

enum class EnemyState {
    Idle,
    Patrolling,
    Chasing,
    Attacking
};

class Projectile;  // Forward declaration
class BloodEffect;  // Forward declaration

class Enemy {
public:
    Enemy(float x, float y, EnemyType type = EnemyType::Skeleton);
    ~Enemy();

    void draw() const;
    void loadTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames);
    void updateAnimation(float deltaTime);
    void update(float deltaTime, float playerX, float playerY, const class Tilemap& tilemap);
    void move(float dx, float dy);
    void shootProjectile(float targetX, float targetY, std::vector<Projectile>& projectiles);
    
    // Position getters
    float getX() const { return x; }
    float getY() const { return y; }
    
    // Rectangle collision accessors (similar to Player)
    float getLeft() const { return x - boundingBoxOffsetX; }
    float getRight() const { return x - boundingBoxOffsetX + boundingBoxWidth; }
    float getTop() const { return y - boundingBoxOffsetY; }
    float getBottom() const { return y - boundingBoxOffsetY + boundingBoxHeight; }
    float getBoundingBoxWidth() const { return boundingBoxWidth; }
    float getBoundingBoxHeight() const { return boundingBoxHeight; }
    
    // Enemy type and state
    EnemyType getType() const { return type; }
    EnemyState getState() const { return state; }
    bool isAlive() const { return alive; }
    void setAlive(bool isAlive) { alive = isAlive; }
    
    // Health system
    int getMaxHealth() const { return maxHealth; }
    int getCurrentHealth() const { return currentHealth; }
    void takeDamage(int damage);
    void heal(int amount);
    
    // Blood effect
    bool shouldCreateBloodEffect() const { return !alive && !bloodEffectCreated; }
    void markBloodEffectCreated() { bloodEffectCreated = true; }

private:
    float x, y;
    float boundingBoxWidth = 16.0f;
    float boundingBoxHeight = 16.0f;
    float boundingBoxOffsetX = 8.0f;
    float boundingBoxOffsetY = 8.0f;
    
    unsigned int textureID;
    int frameWidth, frameHeight;
    int textureWidth, textureHeight;
    int totalFrames;
    float animationSpeed, elapsedTime;
    int currentFrame;
    
    EnemyType type;
    EnemyState state;
    bool alive;
    bool bloodEffectCreated;  // Track if blood effect has been created
    
    // Movement and AI
    float moveSpeed = 50.0f;  // Slower than player
    float patrolRadius = 100.0f;
    float chaseRadius = 150.0f;
    float startX, startY;  // Original position for patrolling
    float patrolTimer = 0.0f;
    float patrolDuration = 3.0f;  // How long to patrol in one direction
    float currentPatrolDirection = 1.0f;  // 1.0f or -1.0f
    
    // Random movement
    float randomMoveTimer = 0.0f;
    float randomMoveDuration = 2.0f;
    float randomMoveX = 0.0f;
    float randomMoveY = 0.0f;
    
    // Shooting
    float shootCooldown = 0.0f;
    float shootInterval = 2.0f;  // Shoot every 2 seconds
    float shootRange = 200.0f;
    
    // Random number generation
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> randomDir;
    
    // Health system
    int maxHealth = 100;
    int currentHealth = 100;
}; 