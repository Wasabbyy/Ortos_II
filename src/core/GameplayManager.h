#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include "player/Player.h"
#include "enemy/Enemy.h"
#include "projectile/Projectile.h"
#include "effects/BloodEffect.h"
#include "input/InputHandler.h"
#include "map/Tilemap.h"
#include "collision/CollisionManager.h"
#include "audio/AudioManager.h"
#include "audio/UIAudioManager.h"
#include "ui/UI.h"
#include "save/SaveManager.h"
#include "save/GameStateManager.h"
#include "save/EnhancedSaveManager.h"

class GameplayManager {
public:
    GameplayManager();
    ~GameplayManager();

    // Initialization
    bool initialize(const std::string& assetPath, AudioManager* audioManager, UIAudioManager* uiAudioManager);
    void setSaveManager(EnhancedSaveManager* saveManager);
    void cleanup();

    // Game state management
    void startNewGame();
    void loadGame(SaveData& saveData, const std::string& assetPath);
    void resetGame();

    // Game loop methods
    void update(float deltaTime, GLFWwindow* window, int windowWidth, int windowHeight);
    void draw(int windowWidth, int windowHeight);
    void drawPaused(int windowWidth, int windowHeight);

    // Game state queries
    bool isGameInitialized() const { return gameInitialized; }
    bool isPlayerAlive() const { return player && player->isAlive(); }
    Player* getPlayer() const { return player; }
    const std::vector<Enemy*>& getEnemies() const { return enemies; }
    const std::vector<Projectile>& getPlayerProjectiles() const { return playerProjectiles; }
    const std::vector<Projectile>& getEnemyProjectiles() const { return enemyProjectiles; }
    const std::vector<BloodEffect*>& getBloodEffects() const { return bloodEffects; }
    Tilemap* getTilemap() const { return tilemap; }
    const std::string& getCurrentLevelPath() const { return currentLevelPath; }
    float getLevelTransitionCooldown() const { return levelTransitionCooldown; }

    // Save/Load operations
    SaveData createSaveData() const;
    void loadGameState(const SaveData& saveData, const std::string& assetPath);
    void updatePlayerStatsInDatabase();

private:
    // Game objects
    Player* player;
    std::vector<Enemy*> enemies;
    std::vector<Projectile> playerProjectiles;
    std::vector<Projectile> enemyProjectiles;
    std::vector<BloodEffect*> bloodEffects;
    InputHandler* inputHandler;
    Tilemap* tilemap;
    CollisionManager collisionManager;

    // Audio managers
    AudioManager* audioManager;
    UIAudioManager* uiAudioManager;
    
    // Save manager
    EnhancedSaveManager* saveManager;

    // Game state
    bool gameInitialized;
    std::string currentLevelPath;
    std::string nextLevelPath;
    float levelTransitionCooldown;
    std::string assetPath;

    // Helper methods
    void initializeGameObjects();
    void createDefaultEnemies();
    void setupProjection();
    void handleLevelTransition();
    void updateGameLogic(float deltaTime, GLFWwindow* window);
    void updateEntities(float deltaTime);
    void handleCollisions();
    void createBloodEffects();
    void cleanupInactiveObjects();
    void drawGameWorld();
    void drawUI(int windowWidth, int windowHeight);
    void drawEntities();
    void drawProjectiles();
    void drawBloodEffects();

    // Level management
    void loadLevel(const std::string& levelPath);
    void respawnEnemies();
    void teleportPlayerToCenter();
};
