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
      currentFrame(0), direction(Direction::Down) {}

Player::~Player() {
    if (textureID) {
        glDeleteTextures(1, &textureID);
    }
}

void Player::move(float dx, float dy) {
    x += dx;
    y += dy;

    if (dx > 0) direction = Direction::Right;
    else if (dx < 0) direction = Direction::Left;
    else if (dy > 0) direction = Direction::Up;
    else if (dy < 0) direction = Direction::Down;
}

void Player::draw() const {
    if (textureID == 0) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Calculate frame position based on current frame index
    int framesPerRow = textureWidth / frameWidth;
    int row = static_cast<int>(direction);
    int col = currentFrame % framesPerRow;

    // Calculate exact texture coordinates without assuming padding
    float u1 = static_cast<float>(col * frameWidth) / textureWidth;
    float v1 = static_cast<float>(row * frameHeight) / textureHeight;
    float u2 = static_cast<float>((col + 1) * frameWidth) / textureWidth;
    float v2 = static_cast<float>((row + 1) * frameHeight) / textureHeight;

    float scale = 3.0f;
    float spriteWidth = 0.1f * scale;
    float spriteHeight = 0.1f * scale;

    glBegin(GL_QUADS);
    glTexCoord2f(u1, v2); glVertex2f(x - spriteWidth, y - spriteHeight); // Bottom-left
    glTexCoord2f(u2, v2); glVertex2f(x + spriteWidth, y - spriteHeight); // Bottom-right
    glTexCoord2f(u2, v1); glVertex2f(x + spriteWidth, y + spriteHeight); // Top-right
    glTexCoord2f(u1, v1); glVertex2f(x - spriteWidth, y + spriteHeight); // Top-left
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
    if (!isMoving) {
        currentFrame = 0; // Reset to the first frame when not moving
        elapsedTime = 0.0f; // Reset elapsed time
        return;
    }

    elapsedTime += deltaTime;
    if (elapsedTime >= animationSpeed) {
        elapsedTime -= animationSpeed;
        currentFrame = (currentFrame + 1) % totalFrames;
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
