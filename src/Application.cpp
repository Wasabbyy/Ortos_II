#include <GLFW/glfw3.h>
#include "Player.h"
#include "InputHandler.h"
#include "TileMap.h"
#include <iostream>
#include "nlohmann/json.hpp"
#include <stb_image.h>
using json = nlohmann::json;

int main() {
    if (!glfwInit()) {
        return -1;
    }

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Ortos II", primaryMonitor, nullptr);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
    glfwMakeContextCurrent(window);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); 
    glEnable(GL_TEXTURE_2D); 

    // Set up viewport and orthographic projection
    int windowWidth = 1920;
    int windowHeight = 1080;
    glfwSetWindowSize(window, windowWidth, windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    Player player;
    stbi_set_flip_vertically_on_load(true);
    player.loadTexture("../assets/graphic/Vampire_Walk.png", 64, 64, 4);
    player.loadIdleTexture("../assets/graphic/Vampire_Idle.png", 64, 64, 2); 
    stbi_set_flip_vertically_on_load(false);
    InputHandler inputHandler;

    float lastTime = glfwGetTime();
    Tilemap level;

    // Load the map
    if (!level.loadFromJSON("../assets/maps/test.json")) {
        std::cerr << "Failed to load map from JSON." << std::endl;
        return -1;
    }

    // Set up projection to match tilemap size
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float mapWidth = level.getWidthInTiles() * level.getTileWidth();
    float mapHeight = level.getHeightInTiles() * level.getTileHeight();
    glOrtho(0.0, mapWidth, mapHeight, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT);

        inputHandler.processInput(window, player, deltaTime, level);
    
        level.draw();
        player.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}