#include "InputHandler.h"
#include "Player.h"
#include "Projectile.h"
#include "TileMap.h"
#include <cmath>
#include <iostream>
#include <spdlog/spdlog.h>

void InputHandler::processInput(GLFWwindow* window, Player& player, float deltaTime, const Tilemap& tilemap, std::vector<Projectile>& projectiles) {
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

    // Shooting with arrow keys
    float shootX = 0.0f, shootY = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) shootY -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) shootY += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) shootX -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) shootX += 1.0f;
    
    if (shootX != 0.0f || shootY != 0.0f) {
        // Normalize shooting direction
        float length = std::sqrt(shootX * shootX + shootY * shootY);
        if (length > 0.0f) {
            shootX /= length;
            shootY /= length;
        }
        
        // Calculate target position (100 pixels away from player)
        float targetX = player.getX() + shootX * 100.0f;
        float targetY = player.getY() + shootY * 100.0f;
        
        player.shootProjectile(targetX, targetY, projectiles);
    }

    // Throttle player position logging to once every 0.5 seconds
    static float lastLogTime = 0.0f;
    float currentLogTime = glfwGetTime();
    if (currentLogTime - lastLogTime > 0.5f) {
       // spdlog::info("Player Position: ({}, {})", player.getX(), player.getY());
        lastLogTime = currentLogTime;
    }

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