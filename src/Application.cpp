#include <GLFW/glfw3.h>
#include "Player.h"
#include "InputHandler.h"
#include "TileMap.h"
#include <iostream>
#include "nlohmann/json.hpp"
#include <stb_image.h>
#include <spdlog/spdlog.h>
using json = nlohmann::json;

int main() {
    // Initialize logger
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    spdlog::info("Starting Ortos II application");
    if (!glfwInit()) {
        spdlog::error("Failed to initialize GLFW");
        return -1;
    }

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Ortos II", primaryMonitor, nullptr);

    if (!window) {
        spdlog::error("Failed to create GLFW window");
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
    Tilemap tilemap;
    if (!tilemap.loadTilesetTexture("../assets/maps/catacombs.png", 16, 16)) {
        spdlog::error("Failed to load tileset texture");
        return -1;
    }
    if (!tilemap.loadFromJSON("../assets/maps/test.json")) {
        spdlog::error("Failed to load map from JSON.");
        return -1;
    }

    // Set up projection to match tilemap size
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float mapWidth = tilemap.getWidthInTiles() * tilemap.getTileWidth();
    float mapHeight = tilemap.getHeightInTiles() * tilemap.getTileHeight();
    glOrtho(0.0, mapWidth, mapHeight, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT);

        inputHandler.processInput(window, player, deltaTime, tilemap);
    
        tilemap.draw();
        player.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    spdlog::info("Shutting down Ortos II application");
    glfwTerminate();
    return 0;
}