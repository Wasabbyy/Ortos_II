#pragma once
#include <string>

enum class Direction {
    Down = 2,
    Left = 1,
    Right = 0,
    Up = 3
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
    int getFrameWidth() const { return isIdle ? idleFrameWidth : frameWidth; }
    int getFrameHeight() const { return isIdle ? idleFrameHeight : frameHeight; }
    // Rectangle collision accessors
    float getLeft() const { return x - boundingBoxOffsetX; }
    float getRight() const { return x - boundingBoxOffsetX + boundingBoxWidth; }
    float getTop() const { return y - boundingBoxOffsetY; }
    float getBottom() const { return y - boundingBoxOffsetY + boundingBoxHeight; }
    float getBoundingBoxWidth() const { return boundingBoxWidth; }
    float getBoundingBoxHeight() const { return boundingBoxHeight; }

private:
    float x, y;
    float boundingBoxWidth = 40.0f;  // Rectangle width for collision
    float boundingBoxHeight = 40.0f; // Rectangle height for collision
    float boundingBoxOffsetX = 12.0f; // Offset from player center to rectangle (tweak as needed)
    float boundingBoxOffsetY = 24.0f; // Offset from player center to rectangle (tweak as needed)
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
