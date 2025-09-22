#pragma once

#include "SaveSlot.h"
#include "SaveData.h"
#include <vector>
#include <string>

class SaveManager {
public:
    static const int MAX_SAVE_SLOTS = 3;
    
private:
    std::vector<SaveSlot> saveSlots;
    std::string saveDirectory;
    
public:
    // Constructor
    SaveManager(const std::string& saveDir);
    
    // Initialize save slots
    void initialize();
    
    // Update all save slots
    void updateSaveSlots();
    
    // Check if any save slots exist
    bool hasAnySave() const;
    
    // Get save slot by index
    SaveSlot& getSaveSlot(int index);
    const SaveSlot& getSaveSlot(int index) const;
    
    // Get the most recent save slot
    int getMostRecentSaveSlot() const;
    
    // Save game to specific slot
    bool saveGame(const SaveData& saveData, int slotIndex);
    
    // Load game from specific slot
    bool loadGame(SaveData& saveData, int slotIndex);
    
    // Get save slot display information
    std::vector<std::string> getSaveSlotInfo() const;
    
    // Check if save file exists (legacy support)
    bool saveFileExists(const std::string& filename = "savegame.json") const;
    
    // Legacy save/load methods (for backward compatibility)
    bool saveGame(const SaveData& saveData, const std::string& filename = "savegame.json");
    bool loadGame(SaveData& saveData, const std::string& filename = "savegame.json");
};
