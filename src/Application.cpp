#include <GLFW/glfw3.h>
#include "Player.h"
#include "InputHandler.h"

int main() {
    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Ortos II", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    Player player;
    player.loadTexture("../assets/player_down1.png", 64, 64, 8); // Example sprite sheet
    InputHandler inputHandler;

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT);

        inputHandler.processInput(window, player);
        player.updateAnimation(deltaTime);
        player.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}