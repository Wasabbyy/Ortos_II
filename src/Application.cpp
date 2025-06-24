#include <GLFW/glfw3.h>
#include "Player.h"
#include "InputHandler.h"

int main() {
    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1366, 768, "Ortos II", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

// ✅ Add these lines to enable alpha blending
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    Player player;
    player.loadTexture("../assets/Vampire_Walk.png", 64, 64, 4); // Example sprite sheet
    player.loadIdleTexture("../assets/Vampire_Idle.png", 64, 64, 2); 
    InputHandler inputHandler;

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
    
        glClear(GL_COLOR_BUFFER_BIT);
    
        inputHandler.processInput(window, player, deltaTime); // Pass deltaTime here
        player.draw();
    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}