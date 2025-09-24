#pragma once

#include <vector>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class SaveData {
public:
    // Player data
    float playerX = 0.0f;
    float playerY = 0.0f;
    int playerHealth = 100;
    int playerMaxHealth = 100;
    int playerXP = 0;
    int playerMaxXP = 100;
    int playerLevel = 1;
    
    // Enemy data
    std::vector<json> enemies;
    
    // Projectile data
    std::vector<json> playerProjectiles;
    std::vector<json> enemyProjectiles;
    
    // Game state
    std::string currentLevelPath;
    float levelTransitionCooldown = 0.0f;
    
    // Extended player data
    int totalXP = 0;
    int coins = 0;
    int playTime = 0; // in seconds
    int enemiesKilled = 0;
    int deaths = 0;
    
    // Inventory system
    std::vector<json> inventory;
    
    // Timestamp
    std::string saveTime;

    // Default constructor
    SaveData() = default;
    
    // Constructor with parameters
    SaveData(float x, float y, int health, int maxHealth, int xp, int maxXP, int level,
             const std::string& levelPath, float cooldown = 0.0f);
    
    // Serialization methods
    json toJson() const;
    void fromJson(const json& j);
    
    // Utility methods
    void setCurrentTime();
    bool isValid() const;
};
