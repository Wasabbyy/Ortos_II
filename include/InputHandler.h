#pragma once

#include <GLFW/glfw3.h>
#include "Player.h"

class InputHandler {
public:
    void processInput(GLFWwindow* window, Player& player);
};