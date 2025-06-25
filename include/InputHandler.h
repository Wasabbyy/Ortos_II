#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <GLFW/glfw3.h>
#include "Player.h"
#include "TileMap.h"

class InputHandler {
public:
    void processInput(GLFWwindow* window, Player& player, float deltaTime, const Tilemap& tilemap);
};

#endif