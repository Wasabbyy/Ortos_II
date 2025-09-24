#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct PlayerStats {
    int playerId = 1; // Default to 1 for single player
    int level = 1;
    int currentXP = 0;
    int maxXP = 100;
    int totalXP = 0;
    int health = 100;
    int maxHealth = 100;
    float x = 0.0f;
    float y = 0.0f;
    std::string currentLevelPath = "";
    std::string lastSaveTime = "";
    
    // Future expansion fields
    int coins = 0;
    int playTime = 0; // in seconds
    int enemiesKilled = 0;
    int deaths = 0;
};

struct Item {
    int id = 0;
    int playerId = 1;
    std::string name = "";
    std::string type = ""; // weapon, armor, consumable, etc.
    int quantity = 1;
    int value = 0;
    json properties = json::object(); // For item-specific data like damage, defense, etc.
    std::string acquiredTime = "";
};

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();
    
    // Database initialization and management
    bool initialize(const std::string& databasePath = "game_data.db");
    void close();
    bool isInitialized() const { return db != nullptr; }
    
    // Player stats management
    bool savePlayerStats(const PlayerStats& stats);
    bool loadPlayerStats(PlayerStats& stats);
    bool updatePlayerStats(const PlayerStats& stats);
    
    // Item management
    bool addItem(const Item& item);
    bool removeItem(int itemId);
    bool updateItem(const Item& item);
    std::vector<Item> getPlayerItems(int playerId = 1);
    std::vector<Item> getItemsByType(const std::string& type, int playerId = 1);
    Item getItem(int itemId);
    
    // Game state management
    bool saveGameState(const json& gameState);
    json loadGameState();
    
    // Utility methods
    bool createTables();
    bool backupDatabase(const std::string& backupPath);
    bool restoreDatabase(const std::string& backupPath);
    
    // Migration from JSON save system
    bool migrateFromJsonSave(const json& saveData);
    
    // Public utility method
    std::string getCurrentTimestamp();
    
private:
    sqlite3* db;
    std::string databasePath;
    
    // Helper methods
    bool executeQuery(const std::string& query);
    bool executeQueryWithCallback(const std::string& query, 
                                 int (*callback)(void*, int, char**, char**), 
                                 void* data);
    bool tableExists(const std::string& tableName);
};
