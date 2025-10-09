#pragma once
#include <GLFW/glfw3.h>
#include <string>
#include "ui/TextRenderer.h"
#include "ui/AnimatedHealthBar.h"
#include "ui/AnimatedXPBar.h"
#include "ui/RomanNumeralRenderer.h"

class UI {
public:
    // Initialize the UI system with a font
    static bool init(const std::string& fontPath = "assets/fonts/Arial.ttc");
    
    // Initialize animated health bar
    static void initAnimatedHealthBar(const std::string& assetPath);
    
    // Update animated health bar
    static void updateAnimatedHealthBar(float deltaTime);
    
    // Initialize animated XP bar
    static void initAnimatedXPBar(const std::string& assetPath);
    
    // Update animated XP bar
    static void updateAnimatedXPBar(float deltaTime);
    
    // Initialize Roman numeral renderer
    static void initRomanNumeralRenderer(const std::string& assetPath);
    
    // Cleanup
    static void cleanup();
    
    // Load title screen background texture
    static bool loadTitleScreenTexture(const std::string& imagePath);
    
    // Load death screen background texture
    static bool loadDeathScreenTexture(const std::string& imagePath);
    
    static void drawPlayerHealth(int currentHealth, int maxHealth, int windowWidth, int windowHeight);
    static void drawAnimatedPlayerHealth(int currentHealth, int maxHealth, int windowWidth, int windowHeight);
    static void drawEnemyHealthBar(float x, float y, int currentHealth, int maxHealth);
    static void drawHeart(float x, float y, bool filled, float size = 16.0f);
    static void drawXPBar(int currentXP, int maxXP, int windowWidth, int windowHeight);
    static void drawAnimatedXPBar(int currentXP, int maxXP, int windowWidth, int windowHeight);
    static void drawAnimatedXPBarWithState(int xpState, int windowWidth, int windowHeight);
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
    
    // Save slot selection functions
    static void drawSaveSlotMenu(int windowWidth, int windowHeight, int selectedSlot, const std::vector<std::string>& saveSlotInfo);
    static void drawLoadSlotMenu(int windowWidth, int windowHeight, int selectedSlot, const std::vector<std::string>& saveSlotInfo);
    
    // Settings menu function
    static void drawSettingsMenu(int windowWidth, int windowHeight, int selectedOption, float masterVolume, float musicVolume, float sfxVolume);

private:
    static TextRenderer* textRenderer;
    static bool initialized;
    static GLuint titleScreenTextureID;
    static GLuint deathScreenTextureID;
    static AnimatedHealthBar* animatedHealthBar;
    static AnimatedXPBar* animatedXPBar;
    static RomanNumeralRenderer* romanNumeralRenderer;
}; 