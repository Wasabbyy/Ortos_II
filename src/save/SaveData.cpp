#include "SaveData.h"
#include <chrono>
#include <ctime>

SaveData::SaveData(float x, float y, int health, int maxHealth, int xp, int maxXP, int level,
                   const std::string& levelPath, float cooldown)
    : playerX(x), playerY(y), playerHealth(health), playerMaxHealth(maxHealth),
      playerXP(xp), playerMaxXP(maxXP), playerLevel(level),
      currentLevelPath(levelPath), levelTransitionCooldown(cooldown) {
    setCurrentTime();
}

json SaveData::toJson() const {
    json j;
    
    // Player data
    j["player"]["x"] = playerX;
    j["player"]["y"] = playerY;
    j["player"]["health"] = playerHealth;
    j["player"]["maxHealth"] = playerMaxHealth;
    j["player"]["xp"] = playerXP;
    j["player"]["maxXP"] = playerMaxXP;
    j["player"]["level"] = playerLevel;
    
    // Enemy and projectile data
    j["enemies"] = enemies;
    j["playerProjectiles"] = playerProjectiles;
    j["enemyProjectiles"] = enemyProjectiles;
    
    // Game state
    j["gameState"]["currentLevelPath"] = currentLevelPath;
    j["gameState"]["levelTransitionCooldown"] = levelTransitionCooldown;
    j["gameState"]["saveTime"] = saveTime;
    
    return j;
}

void SaveData::fromJson(const json& j) {
    try {
        // Load player data
        playerX = j["player"]["x"];
        playerY = j["player"]["y"];
        playerHealth = j["player"]["health"];
        playerMaxHealth = j["player"]["maxHealth"];
        playerXP = j["player"]["xp"];
        playerMaxXP = j["player"]["maxXP"];
        playerLevel = j["player"]["level"];
        
        // Load enemy and projectile data
        enemies = j["enemies"];
        playerProjectiles = j["playerProjectiles"];
        enemyProjectiles = j["enemyProjectiles"];
        
        // Load game state
        currentLevelPath = j["gameState"]["currentLevelPath"];
        levelTransitionCooldown = j["gameState"]["levelTransitionCooldown"];
        saveTime = j["gameState"]["saveTime"];
    } catch (const std::exception& e) {
        // Reset to default values on error
        *this = SaveData();
        throw std::runtime_error("Failed to parse save data: " + std::string(e.what()));
    }
}

void SaveData::setCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    saveTime = std::ctime(&time_t);
    saveTime.pop_back(); // Remove newline
}

bool SaveData::isValid() const {
    return !currentLevelPath.empty() && !saveTime.empty();
}
