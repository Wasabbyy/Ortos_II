#pragma once
#include <string>

class Player {
private:
    float x, y; // Position of the player
    unsigned int textureID; // Texture ID for the sprite
    int frameWidth, frameHeight; // Dimensions of a single frame
    int currentFrame, totalFrames; // Current frame and total frames in the sprite sheet
    float animationSpeed; // Speed of animation (time per frame)
    float elapsedTime; // Time elapsed since the last frame update

public:
    Player();
    ~Player();
    void move(float dx, float dy);
    void draw() const;
    void loadTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames);
    void updateAnimation(float deltaTime);
};