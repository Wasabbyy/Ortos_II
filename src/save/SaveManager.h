#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "nlohmann/json.hpp"
#include "player/Player.h"
#include "enemy/Enemy.h"
#include "projectile/Projectile.h"
#include "map/TileMap.h"

using json = nlohmann::json;

// Save data structure
struct SaveData {
    // Player data
    float playerX;
    float playerY;
    int playerHealth;
    int playerMaxHealth;
    int playerXP;
    int playerMaxXP;
    int playerLevel;
    
    // Enemy data
    std::vector<json> enemies;
    
    // Projectile data
    std::vector<json> playerProjectiles;
    std::vector<json> enemyProjectiles;
    
    // Game state
    std::string currentLevelPath;
    float levelTransitionCooldown;
    
    // Timestamp
    std::string saveTime;
};

// Save slot information
struct SaveSlot {
    int slotNumber;
    std::string filename;
    std::string saveTime;
    bool exists;
    SaveData data;
};

class SaveManager {
public:
    static const int MAX_SAVE_SLOTS = 3;
    
    SaveManager();
    ~SaveManager() = default;
    
    // Save slot management
    void initializeSaveSlots();
    void updateSaveSlots();
    int getMostRecentSaveSlot();
    bool hasAnySaveFile();
    
    // Save/load operations
    bool saveGame(const SaveData& saveData, const std::string& filename);
    bool loadGame(SaveData& saveData, const std::string& filename);
    bool saveFileExists(const std::string& filename);
    void loadGameState(const SaveData& saveData, Player*& player, std::vector<Enemy*>& enemies, 
                      std::vector<Projectile>& playerProjectiles, std::vector<Projectile>& enemyProjectiles,
                      Tilemap*& tilemap, std::string& currentLevelPath, float& levelTransitionCooldown);
    
    // Get save slot information
    const std::vector<SaveSlot>& getSaveSlots() const { return saveSlots_; }
    const SaveSlot& getSaveSlot(int index) const { return saveSlots_[index]; }
    
private:
    std::vector<SaveSlot> saveSlots_;
    
    // Helper functions
    std::string getCurrentTimeString();
    SaveData createSaveData(Player* player, const std::vector<Enemy*>& enemies,
                           const std::vector<Projectile>& playerProjectiles, 
                           const std::vector<Projectile>& enemyProjectiles,
                           const std::string& currentLevelPath, float levelTransitionCooldown);
};
