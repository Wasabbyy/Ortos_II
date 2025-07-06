#pragma once
#include <string>
#include <vector>
#include <random>

enum class EnemyType {
    Skeleton,
    Zombie,
    Ghost,
    FlyingEye,
    Shroom
};

enum class EnemyState {
    Idle,
    Patrolling,
    Chasing,
    Attacking,
    Dying,   // New: for death animation
    Dead     // New: after death animation
};

class Projectile;  // Forward declaration
class BloodEffect;  // Forward declaration

class Enemy {
public:
    Enemy(float x, float y, EnemyType type = EnemyType::Skeleton);
    ~Enemy();

    void draw() const;
    void loadTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames);
    void loadHitTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames);
    void updateAnimation(float deltaTime);
    void update(float deltaTime, float playerX, float playerY, const class Tilemap& tilemap);
    void move(float dx, float dy);
    void shootProjectile(float targetX, float targetY, std::vector<Projectile>& projectiles);
    void loadDeathTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames); // New
    
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

    bool shouldRemoveAfterDeath() const { return state == EnemyState::Dead && deadTimer >= deadRemoveDelay; }
    bool isDeathJustFinished() const { return state == EnemyState::Dead && deadTimer == 0.0f; }

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
    
    // Hit animation properties
    unsigned int hitTextureID = 0;
    int hitFrameWidth = 0, hitFrameHeight = 0;
    int hitTextureWidth = 0, hitTextureHeight = 0;
    int hitTotalFrames = 4;  // 4 sprites in row 1 as specified
    float hitAnimationSpeed = 0.1f;  // Fast hit animation
    float hitElapsedTime = 0.0f;
    int hitCurrentFrame = 0;
    bool isHitAnimationActive = false;
    float hitAnimationDuration = 0.4f;  // Total duration of hit animation
    float hitAnimationTimer = 0.0f;
    
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
    
    // Direction tracking for sprite flipping
    bool facingRight = true;
    float lastMoveX = 0.0f;

    // Death animation properties
    unsigned int deathTextureID = 0;
    int deathFrameWidth = 0, deathFrameHeight = 0;
    int deathTextureWidth = 0, deathTextureHeight = 0;
    int deathTotalFrames = 4; // 4 sprites in a row
    float deathAnimationSpeed = 0.15f; // Standard speed
    float deathElapsedTime = 0.0f;
    int deathCurrentFrame = 0;
    bool isDeathAnimationActive = false;
    float deathAnimationTimer = 0.0f;
    float deathAnimationDuration = 0.6f; // 4 frames * 0.15s
    float deadTimer = 0.0f; // Time since death animation finished
    float deadRemoveDelay = 3.0f; // Remove enemy after 3 seconds
}; 