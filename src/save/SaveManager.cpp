#include "SaveManager.h"
#include <fstream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

SaveManager::SaveManager(const std::string& saveDir) : saveDirectory(saveDir) {
    saveSlots.reserve(MAX_SAVE_SLOTS);
}

void SaveManager::initialize() {
    saveSlots.clear();
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        saveSlots.emplace_back(i + 1, saveDirectory);
    }
    updateSaveSlots();
}

void SaveManager::updateSaveSlots() {
    for (auto& slot : saveSlots) {
        slot.update();
    }
}

bool SaveManager::hasAnySave() const {
    for (const auto& slot : saveSlots) {
        if (slot.hasSave()) {
            return true;
        }
    }
    return false;
}

SaveSlot& SaveManager::getSaveSlot(int index) {
    if (index < 0 || index >= MAX_SAVE_SLOTS) {
        throw std::out_of_range("Save slot index out of range");
    }
    return saveSlots[index];
}

const SaveSlot& SaveManager::getSaveSlot(int index) const {
    if (index < 0 || index >= MAX_SAVE_SLOTS) {
        throw std::out_of_range("Save slot index out of range");
    }
    return saveSlots[index];
}

int SaveManager::getMostRecentSaveSlot() const {
    int mostRecentSlot = -1;
    std::string mostRecentTime = "";
    
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        if (saveSlots[i].hasSave() && saveSlots[i].saveTime > mostRecentTime) {
            mostRecentTime = saveSlots[i].saveTime;
            mostRecentSlot = i;
        }
    }
    
    return mostRecentSlot;
}

bool SaveManager::saveGame(const SaveData& saveData, int slotIndex) {
    if (slotIndex < 0 || slotIndex >= MAX_SAVE_SLOTS) {
        spdlog::error("Invalid save slot index: {}", slotIndex);
        return false;
    }
    
    return saveSlots[slotIndex].saveToFile(saveData);
}

bool SaveManager::loadGame(SaveData& saveData, int slotIndex) {
    if (slotIndex < 0 || slotIndex >= MAX_SAVE_SLOTS) {
        spdlog::error("Invalid save slot index: {}", slotIndex);
        return false;
    }
    
    if (!saveSlots[slotIndex].hasSave()) {
        spdlog::error("Save slot {} does not exist", slotIndex + 1);
        return false;
    }
    
    if (saveSlots[slotIndex].loadSaveData()) {
        // Copy the loaded data to the saveData parameter
        saveData = saveSlots[slotIndex].data;
        spdlog::info("Successfully loaded save data from slot {}", slotIndex + 1);
        return true;
    }
    
    return false;
}

std::vector<std::string> SaveManager::getSaveSlotInfo() const {
    std::vector<std::string> info;
    info.reserve(MAX_SAVE_SLOTS);
    
    for (const auto& slot : saveSlots) {
        info.push_back(slot.getDisplayInfo());
    }
    
    return info;
}

bool SaveManager::saveFileExists(const std::string& filename) const {
    std::string fullPath = saveDirectory + filename;
    std::ifstream file(fullPath);
    return file.good();
}

bool SaveManager::saveGame(const SaveData& saveData, const std::string& filename) {
    try {
        std::string fullPath = saveDirectory + filename;
        json saveJson = saveData.toJson();
        
        std::ofstream file(fullPath);
        if (!file.is_open()) {
            spdlog::error("Failed to open save file: {}", fullPath);
            return false;
        }
        
        file << saveJson.dump(4); // Pretty print with 4 spaces
        file.close();
        
        spdlog::info("Game saved successfully to: {}", fullPath);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to save game: {}", e.what());
        return false;
    }
}

bool SaveManager::loadGame(SaveData& saveData, const std::string& filename) {
    try {
        std::string fullPath = saveDirectory + filename;
        std::ifstream file(fullPath);
        if (!file.is_open()) {
            spdlog::error("Save file not found: {}", fullPath);
            return false;
        }
        
        json saveJson;
        file >> saveJson;
        file.close();
        
        saveData.fromJson(saveJson);
        spdlog::info("Game loaded successfully from: {}", fullPath);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to load game: {}", e.what());
        return false;
    }
}
