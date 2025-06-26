#include "Player.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <iostream>
#include <spdlog/spdlog.h>

Player::Player()
    : x(12 * 16.0f), // Start position in world coordinates
      y(12 * 16.0f),
      textureID(0),
      frameWidth(0), frameHeight(0),
      textureWidth(0), textureHeight(0),
      totalFrames(1), 
      animationSpeed(0.4f), elapsedTime(0.0f),
      currentFrame(0), direction(Direction::Down),
      isMoving(false), isIdle(true) {
    spdlog::debug("Player created at position ({}, {})", x, y);
}

Player::~Player() {
    // Add cleanup logic if needed
}

void Player::draw() const {
    if ((isIdle && idleTextureID == 0) || (!isIdle && textureID == 0)) return;

    unsigned int texID = isIdle ? idleTextureID : textureID;
    int texWidth = isIdle ? idleTextureWidth : textureWidth;
    int texHeight = isIdle ? idleTextureHeight : textureHeight;
    int frameW = isIdle ? idleFrameWidth : frameWidth;
    int frameH = isIdle ? idleFrameHeight : frameHeight;
    int currentF = isIdle ? idleCurrentFrame : currentFrame;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Calculate texture coordinates
    int framesPerRow = texWidth / frameW;

    // Correct the row mapping for directions
    int row = 0;
    switch (direction) {
        case Direction::Down:
            row = 3; // Assuming Down is the first row
            break;
        case Direction::Left:
            row = 1; // Assuming Left is the second row
            break;
        case Direction::Right:
            row = 0; // Assuming Right is the third row
            break;
        case Direction::Up:
            row = 2; // Assuming Up is the fourth row
            break;
    }

    int col = currentF % framesPerRow;

    float u1 = static_cast<float>(col * frameW) / texWidth;
    float v1 = static_cast<float>(row * frameH) / texHeight;
    float u2 = static_cast<float>((col + 1) * frameW) / texWidth;
    float v2 = static_cast<float>((row + 1) * frameH) / texHeight;

    // Draw player centered on tile
    float drawX = x - frameW / 2.0f;
    float drawY = y - frameH / 2.0f;

    glBegin(GL_QUADS);
    glTexCoord2f(u1, v2); glVertex2f(drawX, drawY);
    glTexCoord2f(u2, v2); glVertex2f(drawX + frameW, drawY);
    glTexCoord2f(u2, v1); glVertex2f(drawX + frameW, drawY + frameH);
    glTexCoord2f(u1, v1); glVertex2f(drawX, drawY + frameH);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // Draw collision rectangle (bounding box) in red, thick and always visible
    glLineWidth(3.0f); // Thicker line
    glDisable(GL_BLEND); // Disable blending for solid color
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glBegin(GL_LINE_LOOP);
    glVertex2f(getLeft(), getTop());
    glVertex2f(getRight(), getTop());
    glVertex2f(getRight(), getBottom());
    glVertex2f(getLeft(), getBottom());
    glEnd();
    glEnable(GL_BLEND); // Restore blending
    glLineWidth(1.0f); // Restore line width
    glColor3f(1.0f, 1.0f, 1.0f); // Reset color to white
}

// Rest of the Player class methods remain the same...

void Player::loadTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames) {
    this->frameWidth = frameWidth;
    this->frameHeight = frameHeight;
    this->totalFrames = totalFrames;

    int width, height, channels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        spdlog::error("Failed to load texture: {}", filePath);
        return;
    }

    spdlog::info("Loaded texture: {} ({}x{})", filePath, width, height);

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

    spdlog::debug("Texture loaded successfully with ID: {}", textureID);
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
    float oldX = x, oldY = y;
    x += dx;
    y += dy;
    spdlog::debug("Player moved from ({}, {}) to ({}, {})", oldX, oldY, x, y);
    if (dx > 0) direction = Direction::Right;
    else if (dx < 0) direction = Direction::Left;
    else if (dy > 0) direction = Direction::Up;
    else if (dy < 0) direction = Direction::Down;
}