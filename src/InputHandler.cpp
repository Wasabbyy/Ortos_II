#include "InputHandler.h"
#include <cmath>
#include <iostream>
#include "spdlog.h"

void InputHandler::processInput(GLFWwindow* window, Player& player, float deltaTime, const Tilemap& tilemap) {
    const float moveSpeed = 150.0f;
    float dx = 0.0f, dy = 0.0f;

    // Movement input
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dy += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dy -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dx -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dx += 1.0f;

    bool isMoving = (dx != 0.0f || dy != 0.0f);

    // Normalize and scale
    if (isMoving) {
        float length = std::sqrt(dx * dx + dy * dy);
        if (length > 0.0f) {
            dx = (dx / length) * moveSpeed * deltaTime;
            dy = (dy / length) * moveSpeed * deltaTime;
        }
        spdlog::debug("Input: dx={}, dy={}, moveSpeed={}, deltaTime={}", dx, dy, moveSpeed, deltaTime);
    }

    // Get player rectangle (bounding box)
    float left = player.getLeft();
    float right = player.getRight();
    float top = player.getTop();
    float bottom = player.getBottom();

    // Calculate new rectangle after movement
    float newLeft = left + dx;
    float newRight = right + dx;
    float newTop = top + dy;
    float newBottom = bottom + dy;

    // Check all four corners for collision after movement
    auto isSolid = [&](float px, float py) {
        int tileX = static_cast<int>(px / tilemap.getTileWidth());
        int tileY = static_cast<int>(py / tilemap.getTileHeight());
        return tilemap.isTileSolid(tileX, tileY);
    };

    bool canMoveX = true;
    if (dx != 0) {
        // Check left or right edge depending on direction
        float testX = dx > 0 ? newRight : newLeft;
        if (isSolid(testX, top) || isSolid(testX, bottom - 1)) {
            canMoveX = false;
        }
    }
    bool canMoveY = true;
    if (dy != 0) {
        // Check top or bottom edge depending on direction
        float testY = dy > 0 ? newBottom : newTop;
        if (isSolid(left, testY) || isSolid(right - 1, testY)) {
            canMoveY = false;
        }
    }

    // Apply movement
    if (canMoveX && canMoveY) {
        spdlog::debug("Moving player diagonally: dx={}, dy={}", dx, dy);
        player.move(dx, dy);
    } else if (canMoveX) {
        spdlog::debug("Moving player horizontally: dx={}", dx);
        player.move(dx, 0);
    } else if (canMoveY) {
        spdlog::debug("Moving player vertically: dy={}", dy);
        player.move(0, dy);
    } else {
        spdlog::debug("Player collision detected, no movement");
    }

    spdlog::info("Player Position: ({}, {})", player.getX(), player.getY());

    // Direction update
    if (isMoving) {
        if (std::fabs(dx) > std::fabs(dy)) {
            player.setDirection(dx > 0 ? Direction::Right : Direction::Left);
        } else {
            player.setDirection(dy > 0 ? Direction::Down : Direction::Up);
        }
    }

    player.updateAnimation(deltaTime, isMoving);

    // Debug visualization of collision points

    
}