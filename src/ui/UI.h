#pragma once
#include <GLFW/glfw3.h>
#include <string>
#include "ui/TextRenderer.h"

class UI {
public:
    // Initialize the UI system with a font
    static bool init(const std::string& fontPath = "assets/fonts/Arial.ttc");
    
    // Cleanup
    static void cleanup();
    
    // Load title screen background texture
    static bool loadTitleScreenTexture(const std::string& imagePath);
    
    // Load death screen background texture
    static bool loadDeathScreenTexture(const std::string& imagePath);
    
    static void drawPlayerHealth(int currentHealth, int maxHealth, int windowWidth, int windowHeight);
    static void drawEnemyHealthBar(float x, float y, int currentHealth, int maxHealth);
    static void drawHeart(float x, float y, bool filled, float size = 16.0f);
    static void drawXPBar(int currentXP, int maxXP, int windowWidth, int windowHeight);
    static void drawLevelIndicator(int level, int windowWidth, int windowHeight);
    
    // Menu functions
    static void drawPixelText(const std::string& text, float x, float y, float scale = 1.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f);
    static void drawText(const std::string& text, float x, float y, float scale = 1.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f);
    static void drawCenteredText(const std::string& text, float x, float y, float scale = 1.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f);
    static void drawMenuButton(const std::string& text, float x, float y, float width, float height, bool isHovered, bool isSelected);
    static void drawMainMenu(int windowWidth, int windowHeight, int selectedOption, bool hasSaveFile = false);
    static bool isMouseOverButton(float mouseX, float mouseY, float buttonX, float buttonY, float buttonWidth, float buttonHeight);
    
    // Death screen function
    static void drawDeathScreen(int windowWidth, int windowHeight, bool respawnButtonHovered, bool exitButtonHovered, int selectedButton);
    
    // Pause screen function
    static void drawPauseScreen(int windowWidth, int windowHeight, int selectedButton);

private:
    static TextRenderer* textRenderer;
    static bool initialized;
    static GLuint titleScreenTextureID;
    static GLuint deathScreenTextureID;
}; 