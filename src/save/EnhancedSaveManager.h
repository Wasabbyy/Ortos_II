#pragma once

#include "SaveManager.h"
#include "SaveData.h"
#include "database/DatabaseManager.h"
#include <memory>

class EnhancedSaveManager : public SaveManager {
public:
    EnhancedSaveManager(const std::string& saveDir);
    ~EnhancedSaveManager();
    
    // Initialize with database support
    void initialize();
    
    // Enhanced save/load with database integration
    bool saveGame(const SaveData& saveData, int slotIndex);
    bool loadGame(SaveData& saveData, int slotIndex);
    
    // Database-specific methods
    bool saveToDatabase(const SaveData& saveData, int slotIndex = 1);
    bool loadFromDatabase(SaveData& saveData, int slotIndex = 1);
    bool migrateJsonToDatabase();
    bool updateTemporaryPlayerStats(const SaveData& saveData);
    
    // Item management
    bool addItem(const std::string& name, const std::string& type, int quantity = 1, int value = 0, const json& properties = json::object());
    bool removeItem(int itemId);
    bool updateItem(int itemId, int quantity);
    std::vector<Item> getPlayerItems();
    std::vector<Item> getItemsByType(const std::string& type);
    
    // Player stats management
    bool updatePlayerStats(int level, int currentXP, int maxXP, int health, int maxHealth, 
                          float x, float y, const std::string& levelPath);
    bool updatePlayerProgress(int totalXP, int coins, int playTime, int enemiesKilled, int deaths);
    
    // Database utilities
    bool backupDatabase(const std::string& backupPath);
    bool restoreDatabase(const std::string& backupPath);
    bool isDatabaseEnabled() const { return databaseManager && databaseManager->isInitialized(); }
    
    // Temporary player management
    bool createTemporaryPlayer(const SaveData& saveData);
    bool makeCurrentPlayerPermanent(int playerId = 1);
    bool deleteTemporaryPlayers();
    bool isCurrentPlayerTemporary();
    
    // Hybrid mode: Use both JSON and database
    void setHybridMode(bool enabled) { hybridMode = enabled; }
    bool isHybridMode() const { return hybridMode; }

private:
    std::unique_ptr<DatabaseManager> databaseManager;
    bool hybridMode = true; // Use both JSON and database by default
    std::string databasePath;
    
    // Helper methods
    PlayerStats saveDataToPlayerStats(const SaveData& saveData);
    void playerStatsToSaveData(const PlayerStats& stats, SaveData& saveData);
    std::vector<Item> inventoryToItems(const std::vector<json>& inventory);
    std::vector<json> itemsToInventory(const std::vector<Item>& items);
};
