#include "InputHandler.h"

void InputHandler::processInput(GLFWwindow* window, Player& player) {
    const float moveSpeed = 0.01f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        player.move(0.0f, moveSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        player.move(0.0f, -moveSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        player.move(-moveSpeed, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        player.move(moveSpeed, 0.0f);
    }
}