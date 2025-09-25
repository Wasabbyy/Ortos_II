#include "EnhancedSaveManager.h"
#include <spdlog/spdlog.h>
#include <filesystem>

EnhancedSaveManager::EnhancedSaveManager(const std::string& saveDir) 
    : SaveManager(saveDir), databasePath(saveDir + "game_data.db") {
    databaseManager = std::make_unique<DatabaseManager>();
}

EnhancedSaveManager::~EnhancedSaveManager() {
    if (databaseManager && databaseManager->isInitialized()) {
        // Clean up temporary players before closing database
        deleteTemporaryPlayers();
        spdlog::info("Cleaned up temporary players in destructor");
        databaseManager->close();
    }
}

void EnhancedSaveManager::initialize() {
    // Initialize base SaveManager
    SaveManager::initialize();
    
    // Initialize database
    if (!databaseManager->initialize(databasePath)) {
        spdlog::error("Failed to initialize database, falling back to JSON-only mode");
        hybridMode = false;
        return;
    }
    
    spdlog::info("Enhanced SaveManager initialized with database support");
    
    // Try to migrate existing JSON saves to database
    if (hasAnySave()) {
        spdlog::info("Found existing JSON saves, attempting migration to database");
        migrateJsonToDatabase();
    }
}

bool EnhancedSaveManager::saveGame(const SaveData& saveData, int slotIndex) {
    bool jsonSuccess = true;
    bool dbSuccess = true;
    
    // Save to JSON (original system)
    if (hybridMode) {
        jsonSuccess = SaveManager::saveGame(saveData, slotIndex);
        if (!jsonSuccess) {
            spdlog::warn("Failed to save to JSON slot {}", slotIndex + 1);
        }
    }
    
    // Save to database
    if (databaseManager && databaseManager->isInitialized()) {
        dbSuccess = saveToDatabase(saveData, slotIndex + 1); // Use slot index + 1 as player ID
        if (!dbSuccess) {
            spdlog::warn("Failed to save to database");
        }
        
        // Make player permanent when saving to a slot
        if (dbSuccess) {
            makeCurrentPlayerPermanent(slotIndex + 1);
            spdlog::info("Player made permanent after saving to slot {}", slotIndex + 1);
        }
    }
    
    // Return true if at least one save method succeeded
    return jsonSuccess || dbSuccess;
}

bool EnhancedSaveManager::loadGame(SaveData& saveData, int slotIndex) {
    // Try to load from database first (more reliable)
    if (databaseManager && databaseManager->isInitialized()) {
        if (loadFromDatabase(saveData, slotIndex + 1)) { // Use slot index + 1 as player ID
            spdlog::info("Loaded game from database for slot {}", slotIndex + 1);
            return true;
        }
    }
    
    // Fall back to JSON loading
    if (SaveManager::loadGame(saveData, slotIndex)) {
        spdlog::info("Loaded game from JSON slot {}", slotIndex + 1);
        
        // If we have database support, save the loaded data to database
        if (databaseManager && databaseManager->isInitialized()) {
            saveToDatabase(saveData, slotIndex + 1); // Use slot index + 1 as player ID
        }
        
        return true;
    }
    
    spdlog::error("Failed to load game from both database and JSON");
    return false;
}

bool EnhancedSaveManager::saveToDatabase(const SaveData& saveData, int slotIndex) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return false;
    }
    
    try {
        // Convert SaveData to PlayerStats
        PlayerStats stats = saveDataToPlayerStats(saveData);
        
        // Set player ID to slot index
        stats.playerId = slotIndex;
        
        // Save player stats
        if (!databaseManager->savePlayerStats(stats)) {
            spdlog::error("Failed to save player stats to database");
            return false;
        }
        
        // Save inventory items
        std::vector<Item> items = inventoryToItems(saveData.inventory);
        for (const auto& item : items) {
            if (!databaseManager->addItem(item)) {
                spdlog::warn("Failed to save item to database: {}", item.name);
            }
        }
        
        // Save game state
        json gameState = saveData.toJson();
        if (!databaseManager->saveGameState(gameState)) {
            spdlog::warn("Failed to save game state to database");
        }
        
        spdlog::info("Successfully saved game to database");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Exception while saving to database: {}", e.what());
        return false;
    }
}

bool EnhancedSaveManager::loadFromDatabase(SaveData& saveData, int slotIndex) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return false;
    }
    
    try {
        // Load player stats for specific slot
        PlayerStats stats;
        if (!databaseManager->loadPlayerStats(stats, slotIndex)) {
            spdlog::error("Failed to load player stats from database for slot {}", slotIndex);
            return false;
        }
        
        // Convert PlayerStats to SaveData
        playerStatsToSaveData(stats, saveData);
        
        // Load inventory items for specific slot
        std::vector<Item> items = databaseManager->getPlayerItems(slotIndex);
        saveData.inventory = itemsToInventory(items);
        
        // Load game state
        json gameState = databaseManager->loadGameState();
        if (!gameState.empty()) {
            // Merge game state data into saveData
            if (gameState.contains("enemies")) {
                saveData.enemies = gameState["enemies"];
            }
            if (gameState.contains("playerProjectiles")) {
                saveData.playerProjectiles = gameState["playerProjectiles"];
            }
            if (gameState.contains("enemyProjectiles")) {
                saveData.enemyProjectiles = gameState["enemyProjectiles"];
            }
        }
        
        spdlog::info("Successfully loaded game from database");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Exception while loading from database: {}", e.what());
        return false;
    }
}

bool EnhancedSaveManager::migrateJsonToDatabase() {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return false;
    }
    
    try {
        // Try to load from the most recent save slot
        int mostRecentSlot = getMostRecentSaveSlot();
        if (mostRecentSlot >= 0) {
            SaveData saveData;
            if (SaveManager::loadGame(saveData, mostRecentSlot)) {
                return databaseManager->migrateFromJsonSave(saveData.toJson());
            }
        }
        
        spdlog::info("No JSON saves found to migrate");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Exception during JSON to database migration: {}", e.what());
        return false;
    }
}

bool EnhancedSaveManager::addItem(const std::string& name, const std::string& type, int quantity, int value, const json& properties) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return false;
    }
    
    Item item;
    item.name = name;
    item.type = type;
    item.quantity = quantity;
    item.value = value;
    item.properties = properties;
    item.acquiredTime = databaseManager->getCurrentTimestamp();
    
    return databaseManager->addItem(item);
}

bool EnhancedSaveManager::removeItem(int itemId) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return false;
    }
    
    return databaseManager->removeItem(itemId);
}

bool EnhancedSaveManager::updateItem(int itemId, int quantity) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return false;
    }
    
    Item item = databaseManager->getItem(itemId);
    if (item.id == 0) {
        spdlog::error("Item not found: {}", itemId);
        return false;
    }
    
    item.quantity = quantity;
    return databaseManager->updateItem(item);
}

std::vector<Item> EnhancedSaveManager::getPlayerItems() {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return std::vector<Item>();
    }
    
    return databaseManager->getPlayerItems();
}

std::vector<Item> EnhancedSaveManager::getItemsByType(const std::string& type) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return std::vector<Item>();
    }
    
    return databaseManager->getItemsByType(type);
}

bool EnhancedSaveManager::updatePlayerStats(int level, int currentXP, int maxXP, int health, int maxHealth, 
                                           float x, float y, const std::string& levelPath) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return false;
    }
    
    PlayerStats stats;
    if (!databaseManager->loadPlayerStats(stats)) {
        spdlog::warn("Failed to load existing player stats, creating new");
    }
    
    stats.level = level;
    stats.currentXP = currentXP;
    stats.maxXP = maxXP;
    stats.health = health;
    stats.maxHealth = maxHealth;
    stats.x = x;
    stats.y = y;
    stats.currentLevelPath = levelPath;
    stats.lastSaveTime = databaseManager->getCurrentTimestamp();
    
    return databaseManager->savePlayerStats(stats);
}

bool EnhancedSaveManager::updatePlayerProgress(int totalXP, int coins, int playTime, int enemiesKilled, int deaths) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return false;
    }
    
    PlayerStats stats;
    if (!databaseManager->loadPlayerStats(stats)) {
        spdlog::warn("Failed to load existing player stats, creating new");
    }
    
    stats.totalXP = totalXP;
    stats.coins = coins;
    stats.playTime = playTime;
    stats.enemiesKilled = enemiesKilled;
    stats.deaths = deaths;
    stats.lastSaveTime = databaseManager->getCurrentTimestamp();
    
    return databaseManager->savePlayerStats(stats);
}

bool EnhancedSaveManager::backupDatabase(const std::string& backupPath) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return false;
    }
    
    return databaseManager->backupDatabase(backupPath);
}

bool EnhancedSaveManager::restoreDatabase(const std::string& backupPath) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return false;
    }
    
    return databaseManager->restoreDatabase(backupPath);
}

PlayerStats EnhancedSaveManager::saveDataToPlayerStats(const SaveData& saveData) {
    PlayerStats stats;
    stats.level = saveData.playerLevel;
    stats.currentXP = saveData.playerXP;
    stats.maxXP = saveData.playerMaxXP;
    stats.health = saveData.playerHealth;
    stats.maxHealth = saveData.playerMaxHealth;
    stats.x = saveData.playerX;
    stats.y = saveData.playerY;
    stats.currentLevelPath = saveData.currentLevelPath;
    stats.lastSaveTime = saveData.saveTime;
    stats.totalXP = saveData.totalXP;
    stats.coins = saveData.coins;
    stats.playTime = saveData.playTime;
    stats.enemiesKilled = saveData.enemiesKilled;
    stats.deaths = saveData.deaths;
    return stats;
}

void EnhancedSaveManager::playerStatsToSaveData(const PlayerStats& stats, SaveData& saveData) {
    saveData.playerLevel = stats.level;
    saveData.playerXP = stats.currentXP;
    saveData.playerMaxXP = stats.maxXP;
    saveData.playerHealth = stats.health;
    saveData.playerMaxHealth = stats.maxHealth;
    saveData.playerX = stats.x;
    saveData.playerY = stats.y;
    saveData.currentLevelPath = stats.currentLevelPath;
    saveData.saveTime = stats.lastSaveTime;
    saveData.totalXP = stats.totalXP;
    saveData.coins = stats.coins;
    saveData.playTime = stats.playTime;
    saveData.enemiesKilled = stats.enemiesKilled;
    saveData.deaths = stats.deaths;
}

std::vector<Item> EnhancedSaveManager::inventoryToItems(const std::vector<json>& inventory) {
    std::vector<Item> items;
    for (const auto& itemJson : inventory) {
        Item item;
        item.name = itemJson.value("name", "");
        item.type = itemJson.value("type", "");
        item.quantity = itemJson.value("quantity", 1);
        item.value = itemJson.value("value", 0);
        item.properties = itemJson.value("properties", json::object());
        item.acquiredTime = itemJson.value("acquiredTime", "");
        items.push_back(item);
    }
    return items;
}

std::vector<json> EnhancedSaveManager::itemsToInventory(const std::vector<Item>& items) {
    std::vector<json> inventory;
    for (const auto& item : items) {
        json itemJson;
        itemJson["name"] = item.name;
        itemJson["type"] = item.type;
        itemJson["quantity"] = item.quantity;
        itemJson["value"] = item.value;
        itemJson["properties"] = item.properties;
        itemJson["acquiredTime"] = item.acquiredTime;
        inventory.push_back(itemJson);
    }
    return inventory;
}

// Temporary player management methods
bool EnhancedSaveManager::createTemporaryPlayer(const SaveData& saveData) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        spdlog::warn("Database not initialized, cannot create temporary player");
        return false;
    }
    
    PlayerStats tempStats = saveDataToPlayerStats(saveData);
    tempStats.isTemporary = true;
    tempStats.playerId = 1; // Temporary players always use ID 1
    
    spdlog::info("Creating temporary player in database");
    bool success = databaseManager->createTemporaryPlayer(tempStats);
    
    if (success) {
        // Also save items for temporary player
        for (const auto& itemJson : saveData.inventory) {
            if (itemJson.contains("name") && itemJson.contains("type")) {
                addItem(itemJson["name"], itemJson["type"], 
                       itemJson.value("quantity", 1), 
                       itemJson.value("value", 0), 
                       itemJson.value("properties", json::object()));
            }
        }
    }
    
    return success;
}

bool EnhancedSaveManager::makeCurrentPlayerPermanent(int playerId) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        spdlog::warn("Database not initialized, cannot make player permanent");
        return false;
    }
    
    spdlog::info("Making player {} permanent", playerId);
    return databaseManager->makePlayerPermanent(playerId);
}

bool EnhancedSaveManager::deleteTemporaryPlayers() {
    if (!databaseManager || !databaseManager->isInitialized()) {
        spdlog::warn("Database not initialized, cannot delete temporary players");
        return false;
    }
    
    spdlog::info("Deleting all temporary players");
    return databaseManager->deleteTemporaryPlayers();
}

bool EnhancedSaveManager::isCurrentPlayerTemporary() {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return false;
    }
    
    return databaseManager->isPlayerTemporary(1); // Default player ID is 1
}

bool EnhancedSaveManager::updateTemporaryPlayerStats(const SaveData& saveData) {
    if (!databaseManager || !databaseManager->isInitialized()) {
        return false;
    }
    
    try {
        // Convert SaveData to PlayerStats
        PlayerStats stats = saveDataToPlayerStats(saveData);
        
        // Ensure this is marked as temporary and uses player ID 1
        stats.isTemporary = true;
        stats.playerId = 1;
        
        // Save player stats
        if (!databaseManager->savePlayerStats(stats)) {
            spdlog::error("Failed to update temporary player stats to database");
            return false;
        }
        
        spdlog::info("Updated temporary player stats in database");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Exception in updateTemporaryPlayerStats: {}", e.what());
        return false;
    }
}
