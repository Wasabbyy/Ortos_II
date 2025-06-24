#pragma once
#include <string>

enum class Direction {
    Down = 0,
    Left = 2,
    Right = 3,
    Up = 1
};

class Player {
public:
    Player();
    ~Player();

    void move(float dx, float dy);
    void draw() const;
    void loadTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames);
    void updateAnimation(float deltaTime, bool isMoving);
    void setDirection(Direction newDirection);
    float getX() const;
    float getY() const;
    void loadIdleTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames);
    void updateIdleAnimation(float deltaTime);

private:
    float x, y;
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
};
