#pragma once

#include "SaveData.h"
#include <string>
#include <vector>

// Forward declarations
class Player;
class Enemy;
class Projectile;
class Tilemap;

enum class EnemyType;

class GameStateManager {
public:
    // Load game state from save data
    static void loadGameState(const SaveData& saveData, Player*& player, 
                             std::vector<Enemy*>& enemies,
                             std::vector<Projectile>& playerProjectiles, 
                             std::vector<Projectile>& enemyProjectiles,
                             std::string& currentLevelPath, 
                             float& levelTransitionCooldown,
                             const std::string& assetPath);
    
    // Create save data from current game state
    static SaveData createSaveData(Player* player,
                                  const std::vector<Enemy*>& enemies,
                                  const std::vector<Projectile>& playerProjectiles,
                                  const std::vector<Projectile>& enemyProjectiles,
                                  const std::string& currentLevelPath,
                                  float levelTransitionCooldown);
    
    // Load enemy textures based on type
    static void loadEnemyTextures(Enemy* enemy, EnemyType type, const std::string& assetPath);
    
    // Load player textures
    static void loadPlayerTextures(Player* player, const std::string& assetPath);
};
