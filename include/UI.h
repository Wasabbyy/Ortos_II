#pragma once
#include <GLFW/glfw3.h>

class UI {
public:
    static void drawPlayerHealth(int currentHealth, int maxHealth, int windowWidth, int windowHeight);
    static void drawEnemyHealthBar(float x, float y, int currentHealth, int maxHealth);
    static void drawHeart(float x, float y, bool filled, float size = 16.0f);
}; 