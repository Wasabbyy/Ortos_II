#include "SaveSlot.h"
#include <fstream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

SaveSlot::SaveSlot(int slotNum, const std::string& basePath)
    : slotNumber(slotNum), filename(basePath + "savegame_slot" + std::to_string(slotNum) + ".json"), exists(false) {
    update();
}

void SaveSlot::update() {
    std::ifstream file(filename);
    exists = file.good();
    file.close();
    
    if (exists) {
        // Load save time from file
        try {
            std::ifstream inFile(filename);
            json saveJson;
            inFile >> saveJson;
            inFile.close();
            saveTime = saveJson["gameState"]["saveTime"];
        } catch (const std::exception& e) {
            spdlog::warn("Failed to read save time from slot {}: {}", slotNumber, e.what());
            saveTime = "Unknown";
        }
    } else {
        saveTime = "";
    }
}

bool SaveSlot::hasSave() const {
    return exists;
}

std::string SaveSlot::getDisplayInfo() const {
    if (exists) {
        return saveTime;
    }
    return "Empty";
}

bool SaveSlot::loadSaveData() {
    if (!exists) {
        spdlog::error("Save slot {} does not exist", slotNumber);
        return false;
    }
    
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            spdlog::error("Failed to open save file: {}", filename);
            return false;
        }
        
        json saveJson;
        file >> saveJson;
        file.close();
        
        data.fromJson(saveJson);
        spdlog::info("Loaded save data from slot {}", slotNumber);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to load save data from slot {}: {}", slotNumber, e.what());
        return false;
    }
}

bool SaveSlot::saveToFile(const SaveData& saveData) {
    try {
        json saveJson = saveData.toJson();
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            spdlog::error("Failed to open save file for writing: {}", filename);
            return false;
        }
        
        file << saveJson.dump(4); // Pretty print with 4 spaces
        file.close();
        
        // Update slot information
        update();
        spdlog::info("Saved game to slot {}", slotNumber);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to save game to slot {}: {}", slotNumber, e.what());
        return false;
    }
}

void SaveSlot::clear() {
    if (exists) {
        std::remove(filename.c_str());
        exists = false;
        saveTime = "";
        data = SaveData();
        spdlog::info("Cleared save slot {}", slotNumber);
    }
}
