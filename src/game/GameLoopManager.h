#pragma once

#include <GLFW/glfw3.h>
#include <vector>
#include "game/GameStateManager.h"
#include "save/SaveManager.h"
#include "input/InputManager.h"
#include "player/Player.h"
#include "enemy/Enemy.h"
#include "projectile/Projectile.h"
#include "map/TileMap.h"
#include "effects/BloodEffect.h"
#include "audio/AudioManager.h"
#include "audio/UIAudioManager.h"
#include "input/InputHandler.h"

class GameLoopManager {
public:
    GameLoopManager(GLFWwindow* window, AudioManager& audioManager, UIAudioManager& uiAudioManager);
    ~GameLoopManager();
    
    // Main game loop
    void run();
    
    // Game initialization
    bool initializeGame();
    void cleanupGame();
    
private:
    GLFWwindow* window_;
    AudioManager& audioManager_;
    UIAudioManager& uiAudioManager_;
    
    // Managers
    GameStateManager stateManager_;
    SaveManager saveManager_;
    InputManager inputManager_;
    
    // Game objects
    Player* player_;
    std::vector<Enemy*> enemies_;
    std::vector<Projectile> playerProjectiles_;
    std::vector<Projectile> enemyProjectiles_;
    Tilemap* tilemap_;
    std::vector<BloodEffect> bloodEffects_;
    InputHandler inputHandler_;
    
    // Game state variables
    std::string currentLevelPath_;
    float levelTransitionCooldown_;
    bool hasSaveFile_;
    
    // Rendering
    void render();
    void renderMenu();
    void renderPauseScreen();
    void renderDeathScreen();
    void renderSaveSlotMenu();
    void renderLoadSlotMenu();
    void renderGameplay();
    
    // Game logic
    void updateGameplay();
    void handleMenuLogic();
    void handlePauseLogic();
    void handleDeathLogic();
    void handleSaveSlotLogic();
    void handleLoadSlotLogic();
    
    // Helper functions
    void startNewGame();
    void resetGameState();
    std::vector<std::string> generateSaveSlotInfo();
    std::vector<std::string> generateLoadSlotInfo();
};
