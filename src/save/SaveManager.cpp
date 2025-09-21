#include "save/SaveManager.h"
#include <spdlog/spdlog.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>

SaveManager::SaveManager() {
    initializeSaveSlots();
    updateSaveSlots();
}

void SaveManager::initializeSaveSlots() {
    saveSlots_.resize(MAX_SAVE_SLOTS);
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        saveSlots_[i].slotNumber = i + 1;
        saveSlots_[i].filename = "saves/savegame_slot" + std::to_string(i + 1) + ".json";
        saveSlots_[i].exists = false;
        saveSlots_[i].saveTime = "";
    }
}

void SaveManager::updateSaveSlots() {
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        std::ifstream file(saveSlots_[i].filename);
        saveSlots_[i].exists = file.good();
        file.close();
        
        if (saveSlots_[i].exists) {
            // Load save time from file
            try {
                std::ifstream inFile(saveSlots_[i].filename);
                json saveJson;
                inFile >> saveJson;
                inFile.close();
                saveSlots_[i].saveTime = saveJson["gameState"]["saveTime"];
            } catch (...) {
                saveSlots_[i].saveTime = "Unknown";
            }
        }
    }
}

int SaveManager::getMostRecentSaveSlot() {
    int mostRecentSlot = -1;
    std::string mostRecentTime = "";
    
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        if (saveSlots_[i].exists && saveSlots_[i].saveTime > mostRecentTime) {
            mostRecentTime = saveSlots_[i].saveTime;
            mostRecentSlot = i;
        }
    }
    
    return mostRecentSlot;
}

bool SaveManager::hasAnySaveFile() {
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        if (saveSlots_[i].exists) {
            return true;
        }
    }
    return false;
}

std::string SaveManager::getCurrentTimeString() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

SaveData SaveManager::createSaveData(Player* player, const std::vector<Enemy*>& enemies,
                                   const std::vector<Projectile>& playerProjectiles, 
                                   const std::vector<Projectile>& enemyProjectiles,
                                   const std::string& currentLevelPath, float levelTransitionCooldown) {
    SaveData saveData;
    
    // Save player data
    saveData.playerX = player->getX();
    saveData.playerY = player->getY();
    saveData.playerHealth = player->getCurrentHealth();
    saveData.playerMaxHealth = player->getMaxHealth();
    saveData.playerXP = player->getCurrentXP();
    saveData.playerMaxXP = player->getMaxXP();
    saveData.playerLevel = player->getLevel();
    
    // Save enemy data
    saveData.enemies.clear();
    for (auto* enemy : enemies) {
        json enemyData;
        enemyData["x"] = enemy->getX();
        enemyData["y"] = enemy->getY();
        enemyData["currentHealth"] = enemy->getCurrentHealth();
        enemyData["maxHealth"] = enemy->getMaxHealth();
        enemyData["alive"] = enemy->isAlive();
        enemyData["type"] = static_cast<int>(enemy->getType());
        enemyData["state"] = static_cast<int>(enemy->getState());
        saveData.enemies.push_back(enemyData);
    }
    
    // Save projectile data
    saveData.playerProjectiles.clear();
    for (const auto& projectile : playerProjectiles) {
        json projectileData;
        projectileData["x"] = projectile.getPosition().x;
        projectileData["y"] = projectile.getPosition().y;
        projectileData["velocityX"] = projectile.getVelocity().x;
        projectileData["velocityY"] = projectile.getVelocity().y;
        projectileData["active"] = projectile.isActive();
        saveData.playerProjectiles.push_back(projectileData);
    }
    
    saveData.enemyProjectiles.clear();
    for (const auto& projectile : enemyProjectiles) {
        json projectileData;
        projectileData["x"] = projectile.getPosition().x;
        projectileData["y"] = projectile.getPosition().y;
        projectileData["velocityX"] = projectile.getVelocity().x;
        projectileData["velocityY"] = projectile.getVelocity().y;
        projectileData["active"] = projectile.isActive();
        saveData.enemyProjectiles.push_back(projectileData);
    }
    
    // Save game state
    saveData.currentLevelPath = currentLevelPath;
    saveData.levelTransitionCooldown = levelTransitionCooldown;
    saveData.saveTime = getCurrentTimeString();
    
    return saveData;
}

bool SaveManager::saveGame(const SaveData& saveData, const std::string& filename) {
    try {
        json saveJson;
        
        // Player data
        saveJson["player"]["x"] = saveData.playerX;
        saveJson["player"]["y"] = saveData.playerY;
        saveJson["player"]["health"] = saveData.playerHealth;
        saveJson["player"]["maxHealth"] = saveData.playerMaxHealth;
        saveJson["player"]["xp"] = saveData.playerXP;
        saveJson["player"]["maxXP"] = saveData.playerMaxXP;
        saveJson["player"]["level"] = saveData.playerLevel;
        
        // Enemy data
        saveJson["enemies"] = saveData.enemies;
        
        // Projectile data
        saveJson["playerProjectiles"] = saveData.playerProjectiles;
        saveJson["enemyProjectiles"] = saveData.enemyProjectiles;
        
        // Game state
        saveJson["gameState"]["currentLevelPath"] = saveData.currentLevelPath;
        saveJson["gameState"]["levelTransitionCooldown"] = saveData.levelTransitionCooldown;
        saveJson["gameState"]["saveTime"] = saveData.saveTime;
        
        // Ensure saves directory exists
        std::filesystem::create_directories("saves");
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            spdlog::error("Failed to open save file: {}", filename);
            return false;
        }
        
        file << saveJson.dump(4);
        file.close();
        
        spdlog::info("Game saved successfully to: {}", filename);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to save game: {}", e.what());
        return false;
    }
}

bool SaveManager::loadGame(SaveData& saveData, const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            spdlog::error("Failed to open save file: {}", filename);
            return false;
        }
        
        json saveJson;
        file >> saveJson;
        file.close();
        
        // Load player data
        saveData.playerX = saveJson["player"]["x"];
        saveData.playerY = saveJson["player"]["y"];
        saveData.playerHealth = saveJson["player"]["health"];
        saveData.playerMaxHealth = saveJson["player"]["maxHealth"];
        saveData.playerXP = saveJson["player"]["xp"];
        saveData.playerMaxXP = saveJson["player"]["maxXP"];
        saveData.playerLevel = saveJson["player"]["level"];
        
        // Load enemy data
        saveData.enemies = saveJson["enemies"];
        
        // Load projectile data
        saveData.playerProjectiles = saveJson["playerProjectiles"];
        saveData.enemyProjectiles = saveJson["enemyProjectiles"];
        
        // Load game state
        saveData.currentLevelPath = saveJson["gameState"]["currentLevelPath"];
        saveData.levelTransitionCooldown = saveJson["gameState"]["levelTransitionCooldown"];
        saveData.saveTime = saveJson["gameState"]["saveTime"];
        
        spdlog::info("Game loaded successfully from: {}", filename);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to load game: {}", e.what());
        return false;
    }
}

bool SaveManager::saveFileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

void SaveManager::loadGameState(const SaveData& saveData, Player*& player, std::vector<Enemy*>& enemies, 
                               std::vector<Projectile>& playerProjectiles, std::vector<Projectile>& enemyProjectiles,
                               Tilemap*& tilemap, std::string& currentLevelPath, float& levelTransitionCooldown) {
    // Clear existing game objects
    delete player;
    player = nullptr;
    
    for (auto* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
    
    playerProjectiles.clear();
    enemyProjectiles.clear();
    
    delete tilemap;
    tilemap = nullptr;
    
    // Create new player
    player = new Player();
    player->setPosition(saveData.playerX, saveData.playerY);
    player->setHealth(saveData.playerHealth);
    player->setMaxHealth(saveData.playerMaxHealth);
    player->setXP(saveData.playerXP);
    player->setMaxXP(saveData.playerMaxXP);
    player->setLevel(saveData.playerLevel);
    
    // Create enemies from save data
    for (const auto& enemyData : saveData.enemies) {
        EnemyType type = static_cast<EnemyType>(enemyData["type"]);
        Enemy* enemy = nullptr;
        
        switch (type) {
            case EnemyType::FlyingEye:
                enemy = new Enemy(EnemyType::FlyingEye);
                break;
            case EnemyType::Shroom:
                enemy = new Enemy(EnemyType::Shroom);
                break;
            default:
                spdlog::warn("Unknown enemy type: {}", static_cast<int>(type));
                continue;
        }
        
        if (enemy) {
            enemy->setPosition(enemyData["x"], enemyData["y"]);
            enemy->setCurrentHealth(enemyData["currentHealth"]);
            enemy->setMaxHealth(enemyData["maxHealth"]);
            enemy->setAlive(enemyData["alive"]);
            enemy->setState(static_cast<EnemyState>(enemyData["state"]));
            enemies.push_back(enemy);
        }
    }
    
    // Create projectiles from save data
    for (const auto& projectileData : saveData.playerProjectiles) {
        Projectile projectile;
        projectile.setPosition(projectileData["x"], projectileData["y"]);
        projectile.setVelocity(projectileData["velocityX"], projectileData["velocityY"]);
        projectile.setActive(projectileData["active"]);
        playerProjectiles.push_back(projectile);
    }
    
    for (const auto& projectileData : saveData.enemyProjectiles) {
        Projectile projectile;
        projectile.setPosition(projectileData["x"], projectileData["y"]);
        projectile.setVelocity(projectileData["velocityX"], projectileData["velocityY"]);
        projectile.setActive(projectileData["active"]);
        enemyProjectiles.push_back(projectile);
    }
    
    // Load tilemap
    tilemap = new Tilemap();
    if (!tilemap->loadFromJSON(saveData.currentLevelPath)) {
        spdlog::warn("Failed to load saved level: {}, falling back to default", saveData.currentLevelPath);
        if (!tilemap->loadFromJSON("assets/levels/level1.json")) {
            spdlog::error("Failed to load fallback level");
        }
    }
    
    currentLevelPath = saveData.currentLevelPath;
    levelTransitionCooldown = saveData.levelTransitionCooldown;
    
    spdlog::info("Game state loaded successfully");
}
