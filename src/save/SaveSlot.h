#pragma once

#include "SaveData.h"
#include <string>

class SaveSlot {
public:
    int slotNumber;
    std::string filename;
    std::string saveTime;
    bool exists;
    SaveData data;

    // Constructor
    SaveSlot(int slotNum, const std::string& basePath);
    
    // Update slot information
    void update();
    
    // Check if slot exists
    bool hasSave() const;
    
    // Get display information
    std::string getDisplayInfo() const;
    
    // Load save data from file
    bool loadSaveData();
    
    // Save data to file
    bool saveToFile(const SaveData& saveData);
    
    // Clear slot
    void clear();
};
