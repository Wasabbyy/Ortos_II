#include "Player.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <iostream>

Player::Player()
    : x(0.0f), y(0.0f), textureID(0), frameWidth(0), frameHeight(0),
      currentFrame(0), totalFrames(1), animationSpeed(0.1f), elapsedTime(0.0f) {}

Player::~Player() {
    if (textureID) {
        glDeleteTextures(1, &textureID);
    }
}

void Player::move(float dx, float dy) {
    x += dx;
    y += dy;
}

void Player::draw() const {
    if (textureID) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Calculate texture coordinates for the current frame
        float u = (currentFrame % (1024 / frameWidth)) * (float(frameWidth) / 1024.0f);
        float v = (currentFrame / (1024 / frameWidth)) * (float(frameHeight) / 1024.0f);
        float uWidth = float(frameWidth) / 1024.0f;
        float vHeight = float(frameHeight) / 1024.0f;

        glBegin(GL_QUADS);
        glTexCoord2f(u, v + vHeight); glVertex2f(x - 0.1f, y - 0.1f); // Bottom-left
        glTexCoord2f(u + uWidth, v + vHeight); glVertex2f(x + 0.1f, y - 0.1f); // Bottom-right
        glTexCoord2f(u + uWidth, v); glVertex2f(x + 0.1f, y + 0.1f); // Top-right
        glTexCoord2f(u, v); glVertex2f(x - 0.1f, y + 0.1f); // Top-left
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
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

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

void Player::updateAnimation(float deltaTime) {
    elapsedTime += deltaTime;
    if (elapsedTime >= animationSpeed) {
        elapsedTime = 0.0f;
        currentFrame = (currentFrame + 1) % totalFrames;
    }
}