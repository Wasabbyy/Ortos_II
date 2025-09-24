#include "DatabaseManager.h"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <sstream>
#include <ctime>
#include <iomanip>

DatabaseManager::DatabaseManager() : db(nullptr) {
}

DatabaseManager::~DatabaseManager() {
    close();
}

bool DatabaseManager::initialize(const std::string& databasePath) {
    this->databasePath = databasePath;
    
    // Create saves directory if it doesn't exist
    std::filesystem::path dbPath(databasePath);
    std::filesystem::create_directories(dbPath.parent_path());
    
    int rc = sqlite3_open(databasePath.c_str(), &db);
    if (rc) {
        spdlog::error("Can't open database: {}", sqlite3_errmsg(db));
        return false;
    }
    
    spdlog::info("Opened database successfully: {}", databasePath);
    
    // Create tables if they don't exist
    if (!createTables()) {
        spdlog::error("Failed to create database tables");
        return false;
    }
    
    return true;
}

void DatabaseManager::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
        spdlog::info("Database connection closed");
    }
}

bool DatabaseManager::createTables() {
    const std::vector<std::string> createTableQueries = {
        // Player stats table
        R"(CREATE TABLE IF NOT EXISTS player_stats (
            player_id INTEGER PRIMARY KEY DEFAULT 1,
            level INTEGER NOT NULL DEFAULT 1,
            current_xp INTEGER NOT NULL DEFAULT 0,
            max_xp INTEGER NOT NULL DEFAULT 100,
            total_xp INTEGER NOT NULL DEFAULT 0,
            health INTEGER NOT NULL DEFAULT 100,
            max_health INTEGER NOT NULL DEFAULT 100,
            x REAL NOT NULL DEFAULT 0.0,
            y REAL NOT NULL DEFAULT 0.0,
            current_level_path TEXT NOT NULL DEFAULT '',
            last_save_time TEXT NOT NULL DEFAULT '',
            coins INTEGER NOT NULL DEFAULT 0,
            play_time INTEGER NOT NULL DEFAULT 0,
            enemies_killed INTEGER NOT NULL DEFAULT 0,
            deaths INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        ))",
        
        // Items table
        R"(CREATE TABLE IF NOT EXISTS items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            player_id INTEGER NOT NULL DEFAULT 1,
            name TEXT NOT NULL,
            type TEXT NOT NULL,
            quantity INTEGER NOT NULL DEFAULT 1,
            value INTEGER NOT NULL DEFAULT 0,
            properties TEXT NOT NULL DEFAULT '{}',
            acquired_time TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (player_id) REFERENCES player_stats (player_id)
        ))",
        
        // Game state table
        R"(CREATE TABLE IF NOT EXISTS game_state (
            id INTEGER PRIMARY KEY DEFAULT 1,
            state_data TEXT NOT NULL DEFAULT '{}',
            last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        ))"
    };
    
    for (const auto& query : createTableQueries) {
        if (!executeQuery(query)) {
            spdlog::error("Failed to execute query: {}", query);
            return false;
        }
    }
    
    spdlog::info("Database tables created successfully");
    return true;
}

bool DatabaseManager::savePlayerStats(const PlayerStats& stats) {
    std::string query = R"(
        INSERT OR REPLACE INTO player_stats 
        (player_id, level, current_xp, max_xp, total_xp, health, max_health, 
         x, y, current_level_path, last_save_time, coins, play_time, 
         enemies_killed, deaths, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        return false;
    }
    
    // Bind parameters
    sqlite3_bind_int(stmt, 1, stats.playerId);
    sqlite3_bind_int(stmt, 2, stats.level);
    sqlite3_bind_int(stmt, 3, stats.currentXP);
    sqlite3_bind_int(stmt, 4, stats.maxXP);
    sqlite3_bind_int(stmt, 5, stats.totalXP);
    sqlite3_bind_int(stmt, 6, stats.health);
    sqlite3_bind_int(stmt, 7, stats.maxHealth);
    sqlite3_bind_double(stmt, 8, stats.x);
    sqlite3_bind_double(stmt, 9, stats.y);
    sqlite3_bind_text(stmt, 10, stats.currentLevelPath.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, stats.lastSaveTime.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 12, stats.coins);
    sqlite3_bind_int(stmt, 13, stats.playTime);
    sqlite3_bind_int(stmt, 14, stats.enemiesKilled);
    sqlite3_bind_int(stmt, 15, stats.deaths);
    sqlite3_bind_text(stmt, 16, getCurrentTimestamp().c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        spdlog::error("Failed to save player stats: {}", sqlite3_errmsg(db));
        return false;
    }
    
    spdlog::info("Player stats saved successfully");
    return true;
}

bool DatabaseManager::loadPlayerStats(PlayerStats& stats) {
    std::string query = "SELECT * FROM player_stats WHERE player_id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, stats.playerId);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        stats.playerId = sqlite3_column_int(stmt, 0);
        stats.level = sqlite3_column_int(stmt, 1);
        stats.currentXP = sqlite3_column_int(stmt, 2);
        stats.maxXP = sqlite3_column_int(stmt, 3);
        stats.totalXP = sqlite3_column_int(stmt, 4);
        stats.health = sqlite3_column_int(stmt, 5);
        stats.maxHealth = sqlite3_column_int(stmt, 6);
        stats.x = sqlite3_column_double(stmt, 7);
        stats.y = sqlite3_column_double(stmt, 8);
        
        const char* levelPath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        stats.currentLevelPath = levelPath ? levelPath : "";
        
        const char* saveTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        stats.lastSaveTime = saveTime ? saveTime : "";
        
        stats.coins = sqlite3_column_int(stmt, 11);
        stats.playTime = sqlite3_column_int(stmt, 12);
        stats.enemiesKilled = sqlite3_column_int(stmt, 13);
        stats.deaths = sqlite3_column_int(stmt, 14);
        
        sqlite3_finalize(stmt);
        spdlog::info("Player stats loaded successfully");
        return true;
    } else if (rc == SQLITE_DONE) {
        // No player stats found, use defaults
        sqlite3_finalize(stmt);
        spdlog::info("No player stats found, using defaults");
        return true;
    } else {
        spdlog::error("Failed to load player stats: {}", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }
}

bool DatabaseManager::updatePlayerStats(const PlayerStats& stats) {
    return savePlayerStats(stats); // INSERT OR REPLACE handles updates
}

bool DatabaseManager::addItem(const Item& item) {
    std::string query = R"(
        INSERT INTO items (player_id, name, type, quantity, value, properties, acquired_time)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, item.playerId);
    sqlite3_bind_text(stmt, 2, item.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, item.type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, item.quantity);
    sqlite3_bind_int(stmt, 5, item.value);
    sqlite3_bind_text(stmt, 6, item.properties.dump().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, item.acquiredTime.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        spdlog::error("Failed to add item: {}", sqlite3_errmsg(db));
        return false;
    }
    
    spdlog::info("Item added successfully: {}", item.name);
    return true;
}

bool DatabaseManager::removeItem(int itemId) {
    std::string query = "DELETE FROM items WHERE id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, itemId);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        spdlog::error("Failed to remove item: {}", sqlite3_errmsg(db));
        return false;
    }
    
    spdlog::info("Item removed successfully: ID {}", itemId);
    return true;
}

bool DatabaseManager::updateItem(const Item& item) {
    std::string query = R"(
        UPDATE items 
        SET name = ?, type = ?, quantity = ?, value = ?, properties = ?
        WHERE id = ?
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, item.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, item.type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, item.quantity);
    sqlite3_bind_int(stmt, 4, item.value);
    sqlite3_bind_text(stmt, 5, item.properties.dump().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, item.id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        spdlog::error("Failed to update item: {}", sqlite3_errmsg(db));
        return false;
    }
    
    spdlog::info("Item updated successfully: {}", item.name);
    return true;
}

std::vector<Item> DatabaseManager::getPlayerItems(int playerId) {
    std::vector<Item> items;
    std::string query = "SELECT * FROM items WHERE player_id = ? ORDER BY acquired_time DESC";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        return items;
    }
    
    sqlite3_bind_int(stmt, 1, playerId);
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Item item;
        item.id = sqlite3_column_int(stmt, 0);
        item.playerId = sqlite3_column_int(stmt, 1);
        
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item.name = name ? name : "";
        
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        item.type = type ? type : "";
        
        item.quantity = sqlite3_column_int(stmt, 4);
        item.value = sqlite3_column_int(stmt, 5);
        
        const char* properties = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        if (properties) {
            try {
                item.properties = json::parse(properties);
            } catch (const std::exception& e) {
                spdlog::warn("Failed to parse item properties: {}", e.what());
                item.properties = json::object();
            }
        }
        
        const char* acquiredTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        item.acquiredTime = acquiredTime ? acquiredTime : "";
        
        items.push_back(item);
    }
    
    sqlite3_finalize(stmt);
    spdlog::info("Loaded {} items for player {}", items.size(), playerId);
    return items;
}

std::vector<Item> DatabaseManager::getItemsByType(const std::string& type, int playerId) {
    std::vector<Item> items;
    std::string query = "SELECT * FROM items WHERE player_id = ? AND type = ? ORDER BY acquired_time DESC";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        return items;
    }
    
    sqlite3_bind_int(stmt, 1, playerId);
    sqlite3_bind_text(stmt, 2, type.c_str(), -1, SQLITE_STATIC);
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Item item;
        item.id = sqlite3_column_int(stmt, 0);
        item.playerId = sqlite3_column_int(stmt, 1);
        
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item.name = name ? name : "";
        
        const char* itemType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        item.type = itemType ? itemType : "";
        
        item.quantity = sqlite3_column_int(stmt, 4);
        item.value = sqlite3_column_int(stmt, 5);
        
        const char* properties = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        if (properties) {
            try {
                item.properties = json::parse(properties);
            } catch (const std::exception& e) {
                spdlog::warn("Failed to parse item properties: {}", e.what());
                item.properties = json::object();
            }
        }
        
        const char* acquiredTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        item.acquiredTime = acquiredTime ? acquiredTime : "";
        
        items.push_back(item);
    }
    
    sqlite3_finalize(stmt);
    return items;
}

Item DatabaseManager::getItem(int itemId) {
    Item item;
    std::string query = "SELECT * FROM items WHERE id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        return item;
    }
    
    sqlite3_bind_int(stmt, 1, itemId);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        item.id = sqlite3_column_int(stmt, 0);
        item.playerId = sqlite3_column_int(stmt, 1);
        
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item.name = name ? name : "";
        
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        item.type = type ? type : "";
        
        item.quantity = sqlite3_column_int(stmt, 4);
        item.value = sqlite3_column_int(stmt, 5);
        
        const char* properties = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        if (properties) {
            try {
                item.properties = json::parse(properties);
            } catch (const std::exception& e) {
                spdlog::warn("Failed to parse item properties: {}", e.what());
                item.properties = json::object();
            }
        }
        
        const char* acquiredTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        item.acquiredTime = acquiredTime ? acquiredTime : "";
    }
    
    sqlite3_finalize(stmt);
    return item;
}

bool DatabaseManager::saveGameState(const json& gameState) {
    std::string query = R"(
        INSERT OR REPLACE INTO game_state (id, state_data, last_updated)
        VALUES (1, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        return false;
    }
    
    std::string stateJson = gameState.dump();
    sqlite3_bind_text(stmt, 1, stateJson.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, getCurrentTimestamp().c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        spdlog::error("Failed to save game state: {}", sqlite3_errmsg(db));
        return false;
    }
    
    spdlog::info("Game state saved successfully");
    return true;
}

json DatabaseManager::loadGameState() {
    json gameState = json::object();
    std::string query = "SELECT state_data FROM game_state WHERE id = 1";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        return gameState;
    }
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const char* stateData = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (stateData) {
            try {
                gameState = json::parse(stateData);
            } catch (const std::exception& e) {
                spdlog::warn("Failed to parse game state: {}", e.what());
                gameState = json::object();
            }
        }
    }
    
    sqlite3_finalize(stmt);
    return gameState;
}

bool DatabaseManager::migrateFromJsonSave(const json& saveData) {
    try {
        // Extract player data from JSON save
        PlayerStats stats;
        if (saveData.contains("player")) {
            const auto& player = saveData["player"];
            stats.level = player.value("level", 1);
            stats.currentXP = player.value("xp", 0);
            stats.maxXP = player.value("maxXP", 100);
            stats.health = player.value("health", 100);
            stats.maxHealth = player.value("maxHealth", 100);
            stats.x = player.value("x", 0.0f);
            stats.y = player.value("y", 0.0f);
        }
        
        if (saveData.contains("gameState")) {
            const auto& gameState = saveData["gameState"];
            stats.currentLevelPath = gameState.value("currentLevelPath", "");
            stats.lastSaveTime = gameState.value("saveTime", "");
        }
        
        // Save to database
        if (!savePlayerStats(stats)) {
            spdlog::error("Failed to migrate player stats to database");
            return false;
        }
        
        // Save game state
        if (!saveGameState(saveData)) {
            spdlog::error("Failed to migrate game state to database");
            return false;
        }
        
        spdlog::info("Successfully migrated JSON save data to database");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to migrate from JSON save: {}", e.what());
        return false;
    }
}

bool DatabaseManager::backupDatabase(const std::string& backupPath) {
    if (!db) {
        spdlog::error("Database not initialized");
        return false;
    }
    
    sqlite3* backupDb;
    int rc = sqlite3_open(backupPath.c_str(), &backupDb);
    if (rc) {
        spdlog::error("Can't open backup database: {}", sqlite3_errmsg(backupDb));
        return false;
    }
    
    sqlite3_backup* backup = sqlite3_backup_init(backupDb, "main", db, "main");
    if (backup) {
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    }
    
    rc = sqlite3_errcode(backupDb);
    sqlite3_close(backupDb);
    
    if (rc == SQLITE_OK) {
        spdlog::info("Database backed up successfully to: {}", backupPath);
        return true;
    } else {
        spdlog::error("Failed to backup database");
        return false;
    }
}

bool DatabaseManager::restoreDatabase(const std::string& backupPath) {
    if (!std::filesystem::exists(backupPath)) {
        spdlog::error("Backup file does not exist: {}", backupPath);
        return false;
    }
    
    sqlite3* backupDb;
    int rc = sqlite3_open(backupPath.c_str(), &backupDb);
    if (rc) {
        spdlog::error("Can't open backup database: {}", sqlite3_errmsg(backupDb));
        return false;
    }
    
    sqlite3_backup* backup = sqlite3_backup_init(db, "main", backupDb, "main");
    if (backup) {
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    }
    
    rc = sqlite3_errcode(db);
    sqlite3_close(backupDb);
    
    if (rc == SQLITE_OK) {
        spdlog::info("Database restored successfully from: {}", backupPath);
        return true;
    } else {
        spdlog::error("Failed to restore database");
        return false;
    }
}

bool DatabaseManager::executeQuery(const std::string& query) {
    char* errMsg = 0;
    int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        spdlog::error("SQL error: {}", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::executeQueryWithCallback(const std::string& query, 
                                              int (*callback)(void*, int, char**, char**), 
                                              void* data) {
    char* errMsg = 0;
    int rc = sqlite3_exec(db, query.c_str(), callback, data, &errMsg);
    if (rc != SQLITE_OK) {
        spdlog::error("SQL error: {}", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

std::string DatabaseManager::getCurrentTimestamp() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool DatabaseManager::tableExists(const std::string& tableName) {
    std::string query = "SELECT name FROM sqlite_master WHERE type='table' AND name=?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, tableName.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_ROW;
}
