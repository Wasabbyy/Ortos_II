#pragma once
#include <GLFW/glfw3.h>
#include <vector>

class Player;
class Tilemap;
class Projectile;

class InputHandler {
public:
    void processInput(GLFWwindow* window, Player& player, float deltaTime, const Tilemap& tilemap, std::vector<Projectile>& projectiles);
};