#include "Player.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <iostream>

Player::Player()
    : x(0.0f), y(0.0f), textureID(0),
      frameWidth(0), frameHeight(0),
      textureWidth(0), textureHeight(0),
      totalFrames(1), 
      animationSpeed(0.4f), elapsedTime(0.0f),
      currentFrame(0), direction(Direction::Down),
      isMoving(false), isIdle(true) {} // Initialize isIdle

Player::~Player() {
    if (textureID) {
        glDeleteTextures(1, &textureID);
    }
}

void Player::draw() const {
    unsigned int texID;
    int texWidth, texHeight;
    int frameW, frameH;
    int currentF;
    int totalF;

    // Use the isIdle flag to determine which texture to use
    if (isIdle) { // Use idle animation
        if (idleTextureID == 0) return;
        texID = idleTextureID;
        texWidth = idleTextureWidth;
        texHeight = idleTextureHeight;
        frameW = idleFrameWidth;
        frameH = idleFrameHeight;
        currentF = idleCurrentFrame;
        totalF = idleTotalFrames;
    } else { // Use walking animation
        if (textureID == 0) return;
        texID = textureID;
        texWidth = textureWidth;
        texHeight = textureHeight;
        frameW = frameWidth;
        frameH = frameHeight;
        currentF = currentFrame;
        totalF = totalFrames;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Calculate frame position based on current frame index
    int framesPerRow = texWidth / frameW;
    int row = static_cast<int>(direction);
    int col = currentF % framesPerRow;

    // Calculate texture coordinates
    float u1 = static_cast<float>(col * frameW) / texWidth;
    float v1 = static_cast<float>(row * frameH) / texHeight;
    float u2 = static_cast<float>((col + 1) * frameW) / texWidth;
    float v2 = static_cast<float>((row + 1) * frameH) / texHeight;

    float scale = 2.0f;
    float spriteWidth = 0.1f * scale;
    float spriteHeight = 0.1f * scale;

    glBegin(GL_QUADS);
    glTexCoord2f(u1, v2); glVertex2f(x - spriteWidth, y - spriteHeight);
    glTexCoord2f(u2, v2); glVertex2f(x + spriteWidth, y - spriteHeight);
    glTexCoord2f(u2, v1); glVertex2f(x + spriteWidth, y + spriteHeight);
    glTexCoord2f(u1, v1); glVertex2f(x - spriteWidth, y + spriteHeight);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void Player::loadTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames) {
    this->frameWidth = frameWidth;
    this->frameHeight = frameHeight;
    this->totalFrames = totalFrames;

    int width, height, channels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        return;
    }

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
}

void Player::updateAnimation(float deltaTime, bool isMoving) {
    this->isIdle = !isMoving; // Update idle state

    if (isMoving) {
        // Update walking animation
        elapsedTime += deltaTime;
        if (elapsedTime >= animationSpeed) {
            elapsedTime -= animationSpeed;
            currentFrame = (currentFrame + 1) % totalFrames;
        }
        // Reset idle animation when moving starts
        idleCurrentFrame = 0;
        idleElapsedTime = 0.0f;
    } else {
        // Update idle animation when not moving
        updateIdleAnimation(deltaTime);
        // Reset walking animation when idle starts
        currentFrame = 0;
        elapsedTime = 0.0f;
    }
}

void Player::setDirection(Direction newDirection) {
    direction = newDirection;
}
float Player::getX() const {
    return x;
}

float Player::getY() const {
    return y;
}
void Player::loadIdleTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames) {
    this->idleFrameWidth = frameWidth;
    this->idleFrameHeight = frameHeight;
    this->idleTotalFrames = totalFrames;

    int width, height, channels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load idle texture: " << filePath << std::endl;
        return;
    }

    idleTextureWidth = width;
    idleTextureHeight = height;

    glGenTextures(1, &idleTextureID);
    glBindTexture(GL_TEXTURE_2D, idleTextureID);

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
}

void Player::updateIdleAnimation(float deltaTime) {
    idleElapsedTime += deltaTime;
    if (idleElapsedTime >= idleAnimationSpeed) {
        idleElapsedTime -= idleAnimationSpeed;
        idleCurrentFrame = (idleCurrentFrame + 1) % idleTotalFrames;
    }
    
}
// In Player.cpp
void Player::move(float dx, float dy) {
    x += dx;
    y += dy;

    if (dx > 0) direction = Direction::Right;
    else if (dx < 0) direction = Direction::Left;
    else if (dy > 0) direction = Direction::Up;
    else if (dy < 0) direction = Direction::Down;
}