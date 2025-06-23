#include "InputHandler.h"
#include <cmath>    // for sqrt and fabs
#include <iostream> // for std::cout

void InputHandler::processInput(GLFWwindow* window, Player& player, float deltaTime) {
    const float moveSpeed = 0.5f; // units per second

    float dx = 0.0f, dy = 0.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dy += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dy -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dx -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dx += 1.0f;

    bool isMoving = (dx != 0.0f || dy != 0.0f);

    if (isMoving) {
        // Normalize the direction vector
        float length = std::sqrt(dx * dx + dy * dy);
        dx = (dx / length) * moveSpeed * deltaTime;
        dy = (dy / length) * moveSpeed * deltaTime;

        player.move(dx, dy);

        // Set direction based on dominant axis
        if (std::fabs(dx) > std::fabs(dy)) {
            player.setDirection(dx > 0 ? Direction::Right : Direction::Left);
        } else {
            player.setDirection(dy > 0 ? Direction::Up : Direction::Down);
        }

        // 🔧 Debug: print position after movement
        std::cout << "Player Position -> X: " << player.getX()
                  << " | Y: " << player.getY() << std::endl;
    }

    player.updateAnimation(deltaTime, isMoving);
}
