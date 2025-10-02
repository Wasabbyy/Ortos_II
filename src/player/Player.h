#pragma once
#include <string>
#include <vector>

enum class Direction {
    Down = 2,
    Left = 1,
    Right = 0,
    Up = 3
};

class Projectile;  // Forward declaration

class Player {
public:
    Player();
    ~Player();

    void move(float dx, float dy);
    void draw() const;
    void loadTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames);
    void updateAnimation(float deltaTime, bool isMoving);
    void setDirection(Direction newDirection);
    void shootProjectile(float targetX, float targetY, std::vector<Projectile>& projectiles);
    void takeDamage(int damage);
    void heal(int amount);
    float getX() const;
    float getY() const;
    void loadIdleTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames);
    void updateIdleAnimation(float deltaTime);
    int getFrameWidth() const { return isIdle ? idleFrameWidth : frameWidth; }
    int getFrameHeight() const { return isIdle ? idleFrameHeight : frameHeight; }
    // Rectangle collision accessors
    float getLeft() const { return x - boundingBoxOffsetX; }
    float getRight() const { return x - boundingBoxOffsetX + boundingBoxWidth; }
    float getTop() const { return y - boundingBoxOffsetY; }
    float getBottom() const { return y - boundingBoxOffsetY + boundingBoxHeight; }
    float getBoundingBoxWidth() const { return boundingBoxWidth; }
    float getBoundingBoxHeight() const { return boundingBoxHeight; }
    
    // Health system
    int getMaxHealth() const { return maxHealth; }
    int getCurrentHealth() const { return currentHealth; }
    bool isAlive() const { return currentHealth > 0; }
    
    // XP system
    int getCurrentXP() const { return currentXP; }
    int getMaxXP() const { return maxXP; }
    int getLevel() const { return level; }
    int getXPState() const { return xpState; }
    void gainXP(int amount);
    void levelUp();
    void updateXPState();
    
    // Collision state
    void setCollidingWithEnemy(bool colliding) { m_isCollidingWithEnemy = colliding; }
    bool isCollidingWithEnemy() const { return m_isCollidingWithEnemy; }

private:
    float x, y;
    float boundingBoxWidth = 16.0f;   // Much smaller rectangle width for collision
    float boundingBoxHeight = 16.0f;  // Much smaller rectangle height for collision
    float boundingBoxOffsetX = 8.0f;  // Center the rectangle horizontally (16/2 = 8)
    float boundingBoxOffsetY = 8.0f;  // Center the rectangle vertically (16/2 = 8)
    unsigned int textureID;
    int frameWidth, frameHeight;
    int textureWidth, textureHeight;   // ✅ NEW
    int totalFrames;
    int padding;                       // ✅ NEW (defaults to 0)
    float animationSpeed, elapsedTime;
    int currentFrame;
    Direction direction;
    unsigned int idleTextureID = 0;
    int idleFrameWidth = 0;
    int idleFrameHeight = 0;
    int idleTextureWidth = 0;
    int idleTextureHeight = 0;
    int idleTotalFrames = 1;
    float idleAnimationSpeed = 0.5f;
    float idleElapsedTime = 0.0f;
    int idleCurrentFrame = 0;
    bool isMoving;
    bool isIdle; // ✅ NEW: Track if the player is moving
    bool m_isCollidingWithEnemy = false; // Track if player is colliding with an enemy
    
    // Shooting
    float shootCooldown = 0.0f;
    float shootInterval = 0.5f;  // Shoot every 0.5 seconds
    
    // Health system
    int maxHealth = 100;
    int currentHealth = 100;
    
    // XP system
    int currentXP = 0;
    int maxXP = 100;  // Fixed XP needed for each level
    int level = 1;
    int xpState = 0;  // Current XP bar state (0-4 for xpbar_01 to xpbar_05)
};
