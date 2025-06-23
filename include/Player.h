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
};
