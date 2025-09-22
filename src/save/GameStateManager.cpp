#include "GameStateManager.h"
#include "player/Player.h"
#include "enemy/Enemy.h"
#include "projectile/Projectile.h"
#include "map/TileMap.h"
#include <stb_image.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void GameStateManager::loadGameState(const SaveData& saveData, Player*& player, 
                                   std::vector<Enemy*>& enemies,
                                   std::vector<Projectile>& playerProjectiles, 
                                   std::vector<Projectile>& enemyProjectiles,
                                   std::string& currentLevelPath, 
                                   float& levelTransitionCooldown,
                                   const std::string& assetPath) {
    spdlog::info("=== GAMESTATEMANAGER: Starting loadGameState ===");
    spdlog::info("Asset path: {}", assetPath);
    spdlog::info("Player position: {}, {}", saveData.playerX, saveData.playerY);
    spdlog::info("Player health: {}/{}", saveData.playerHealth, saveData.playerMaxHealth);
    spdlog::info("Number of enemies to load: {}", saveData.enemies.size());
    
    if (!player) {
        spdlog::error("Player is null, cannot load game state");
        return;
    }
    
    // Restore player state - we need to create a new player with the saved state
    // since the Player class doesn't have setter methods
    spdlog::info("Deleting existing player and creating new one");
    delete player;
    player = new Player();
    
    // Load player textures
    spdlog::info("Loading player textures");
    loadPlayerTextures(player, assetPath);
    
    // Move player to saved position
    player->move(saveData.playerX - player->getX(), saveData.playerY - player->getY());
    
    // Restore health by healing/damaging as needed
    int healthDiff = saveData.playerHealth - player->getCurrentHealth();
    if (healthDiff > 0) {
        player->heal(healthDiff);
    } else if (healthDiff < 0) {
        player->takeDamage(-healthDiff);
    }
    
    // Restore XP by gaining XP
    int xpDiff = saveData.playerXP - player->getCurrentXP();
    if (xpDiff > 0) {
        player->gainXP(xpDiff);
    }
    
    // Restore game state
    currentLevelPath = saveData.currentLevelPath;
    levelTransitionCooldown = saveData.levelTransitionCooldown;
    
    // Clear existing projectiles
    playerProjectiles.clear();
    enemyProjectiles.clear();
    
    // Clear existing enemies
    spdlog::info("Clearing {} existing enemies", enemies.size());
    for (auto& enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
    
    // Restore enemies from save data
    spdlog::info("Loading {} enemies from save data", saveData.enemies.size());
    for (const auto& enemyData : saveData.enemies) {
        // Create enemy based on saved data
        EnemyType enemyType = static_cast<EnemyType>(enemyData["type"]);
        spdlog::info("Creating enemy of type {} at position ({}, {})", 
                     static_cast<int>(enemyType), enemyData["x"].get<float>(), enemyData["y"].get<float>());
        Enemy* enemy = new Enemy(enemyData["x"], enemyData["y"], enemyType);
        
        // Load appropriate textures based on enemy type
        spdlog::info("Loading textures for enemy type {}", static_cast<int>(enemyType));
        loadEnemyTextures(enemy, enemyType, assetPath);
        
        // Restore enemy state
        enemy->setAlive(enemyData["alive"]);
        // Restore health by taking damage if needed
        int maxHealth = enemyData["maxHealth"].get<int>();
        int currentHealth = enemyData["health"].get<int>();
        int healthDiff = maxHealth - currentHealth;
        if (healthDiff > 0) {
            enemy->takeDamage(healthDiff);
        }
        
        enemies.push_back(enemy);
        spdlog::info("Enemy loaded successfully");
    }
    
    spdlog::info("Game state loaded successfully");
}

SaveData GameStateManager::createSaveData(Player* player,
                                        const std::vector<Enemy*>& enemies,
                                        const std::vector<Projectile>& playerProjectiles,
                                        const std::vector<Projectile>& enemyProjectiles,
                                        const std::string& currentLevelPath,
                                        float levelTransitionCooldown) {
    SaveData saveData;
    
    if (player) {
        saveData.playerX = player->getX();
        saveData.playerY = player->getY();
        saveData.playerHealth = player->getCurrentHealth();
        saveData.playerMaxHealth = player->getMaxHealth();
        saveData.playerXP = player->getCurrentXP();
        saveData.playerMaxXP = player->getMaxXP();
        saveData.playerLevel = player->getLevel();
    }
    
    saveData.currentLevelPath = currentLevelPath;
    saveData.levelTransitionCooldown = levelTransitionCooldown;
    
    // Save current enemies
    for (const auto& enemy : enemies) {
        if (enemy) {
            json enemyData;
            enemyData["x"] = enemy->getX();
            enemyData["y"] = enemy->getY();
            enemyData["health"] = enemy->getCurrentHealth();
            enemyData["maxHealth"] = enemy->getMaxHealth();
            enemyData["alive"] = enemy->isAlive();
            enemyData["type"] = static_cast<int>(enemy->getType());
            enemyData["state"] = static_cast<int>(enemy->getState());
            saveData.enemies.push_back(enemyData);
        }
    }
    
    // Set current time
    saveData.setCurrentTime();
    
    return saveData;
}

void GameStateManager::loadEnemyTextures(Enemy* enemy, EnemyType type, const std::string& assetPath) {
    spdlog::info("Loading textures for enemy type {} with asset path: {}", static_cast<int>(type), assetPath);
    stbi_set_flip_vertically_on_load(true);
    
    if (type == EnemyType::FlyingEye) {
        std::string texturePath = assetPath + "assets/graphic/enemies/flying_eye/flgyingeye.png";
        std::string hitPath = assetPath + "assets/graphic/enemies/flying_eye/Hit_eye.png";
        std::string deathPath = assetPath + "assets/graphic/enemies/flying_eye/Death_eye.png";
        
        spdlog::info("Loading FlyingEye textures: {}, {}, {}", texturePath, hitPath, deathPath);
        enemy->loadTexture(texturePath, 150, 150, 8);
        enemy->loadHitTexture(hitPath, 150, 150, 4);
        enemy->loadDeathTexture(deathPath, 150, 150, 4);
    } else if (type == EnemyType::Shroom) {
        std::string texturePath = assetPath + "assets/graphic/enemies/shroom/shroom.png";
        std::string hitPath = assetPath + "assets/graphic/enemies/shroom/Hit_shroom.png";
        std::string deathPath = assetPath + "assets/graphic/enemies/shroom/Death_shroom.png";
        
        spdlog::info("Loading Shroom textures: {}, {}, {}", texturePath, hitPath, deathPath);
        enemy->loadTexture(texturePath, 150, 150, 8);
        enemy->loadHitTexture(hitPath, 150, 150, 4);
        enemy->loadDeathTexture(deathPath, 150, 150, 4);
    }
    
    stbi_set_flip_vertically_on_load(false);
    spdlog::info("Enemy textures loaded successfully");
}

void GameStateManager::loadPlayerTextures(Player* player, const std::string& assetPath) {
    spdlog::info("Loading player textures with asset path: {}", assetPath);
    stbi_set_flip_vertically_on_load(true);
    
    std::string walkPath = assetPath + "assets/graphic/enemies/vampire/Vampire_Walk.png";
    std::string idlePath = assetPath + "assets/graphic/enemies/vampire/Vampire_Idle.png";
    
    spdlog::info("Loading player textures: {}, {}", walkPath, idlePath);
    player->loadTexture(walkPath, 64, 64, 4);
    player->loadIdleTexture(idlePath, 64, 64, 2);
    stbi_set_flip_vertically_on_load(false);
    spdlog::info("Player textures loaded successfully");
}
