#include "InputHandler.h"
#include <cmath>
#include <iostream>

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
    }

    // Get player dimensions (assuming player is one tile size)
    float playerWidth = static_cast<float>(tilemap.getTileWidth());
    float playerHeight = static_cast<float>(tilemap.getTileHeight());

    // Calculate new position
    float newX = player.getX() + dx;
    float newY = player.getY() + dy;

    // Calculate tile coordinates for all four corners of the player
    int leftTile = static_cast<int>(newX / tilemap.getTileWidth());
    int rightTile = static_cast<int>((newX + playerWidth - 1) / tilemap.getTileWidth());
    int topTile = static_cast<int>(newY / tilemap.getTileHeight());
    int bottomTile = static_cast<int>((newY + playerHeight - 1) / tilemap.getTileHeight());

    // Check bounds
    if (leftTile < 0 || rightTile >= tilemap.getWidthInTiles() ||
        topTile < 0 || bottomTile >= tilemap.getHeightInTiles()) {
        return; // Don't move if out of bounds
    }

    // Check collisions for horizontal movement
    bool canMoveX = true;
    if (dx != 0) {
        int checkY1 = static_cast<int>(player.getY() / tilemap.getTileHeight());
        int checkY2 = static_cast<int>((player.getY() + playerHeight - 1) / tilemap.getTileHeight());
        
        int checkX = dx > 0 ? rightTile : leftTile;
        
        for (int y = checkY1; y <= checkY2; y++) {
            if (tilemap.isTileSolid(checkX, y)) {
                canMoveX = false;
                break;
            }
        }
    }

    // Check collisions for vertical movement
    bool canMoveY = true;
    if (dy != 0) {
        int checkX1 = static_cast<int>(player.getX() / tilemap.getTileWidth());
        int checkX2 = static_cast<int>((player.getX() + playerWidth - 1) / tilemap.getTileWidth());
        
        int checkY = dy > 0 ? bottomTile : topTile;
        
        for (int x = checkX1; x <= checkX2; x++) {
            if (tilemap.isTileSolid(x, checkY)) {
                canMoveY = false;
                break;
            }
        }
    }

    // Apply movement
    if (canMoveX && canMoveY) {
        player.move(dx, dy);
    } else if (canMoveX) {
        player.move(dx, 0);
    } else if (canMoveY) {
        player.move(0, dy);
    }

    // Debug print for player's position
    std::cout << "Player Position: (" << player.getX() << ", " << player.getY() << ")" << std::endl;

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