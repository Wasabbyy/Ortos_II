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
    
    // Extended player data
    j["player"]["totalXP"] = totalXP;
    j["player"]["coins"] = coins;
    j["player"]["playTime"] = playTime;
    j["player"]["enemiesKilled"] = enemiesKilled;
    j["player"]["deaths"] = deaths;
    
    // Inventory
    j["inventory"] = inventory;
    
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
        
        // Load extended player data (with defaults for backward compatibility)
        totalXP = j["player"].value("totalXP", 0);
        coins = j["player"].value("coins", 0);
        playTime = j["player"].value("playTime", 0);
        enemiesKilled = j["player"].value("enemiesKilled", 0);
        deaths = j["player"].value("deaths", 0);
        
        // Load inventory (with default empty array for backward compatibility)
        if (j.contains("inventory")) {
            inventory = j["inventory"];
        }
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
