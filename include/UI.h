#pragma once
#include <GLFW/glfw3.h>
#include <string>

class UI {
public:
    static void drawPlayerHealth(int currentHealth, int maxHealth, int windowWidth, int windowHeight);
    static void drawEnemyHealthBar(float x, float y, int currentHealth, int maxHealth);
    static void drawHeart(float x, float y, bool filled, float size = 16.0f);
    
    // Menu functions
    static void drawPixelText(const std::string& text, float x, float y, float scale = 1.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f);
    static void drawMenuButton(const std::string& text, float x, float y, float width, float height, bool isHovered, bool isSelected);
    static void drawMainMenu(int windowWidth, int windowHeight, int selectedOption);
    static bool isMouseOverButton(float mouseX, float mouseY, float buttonX, float buttonY, float buttonWidth, float buttonHeight);
}; 