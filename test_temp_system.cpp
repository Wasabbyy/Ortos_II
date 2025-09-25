#include <iostream>
#include <memory>
#include "src/database/DatabaseManager.h"
#include "src/save/EnhancedSaveManager.h"
#include "src/save/SaveData.h"

int main() {
    std::cout << "Testing Temporary Player System" << std::endl;
    
    // Initialize Enhanced Save Manager
    EnhancedSaveManager saveManager("saves/");
    saveManager.initialize();
    
    if (!saveManager.isDatabaseEnabled()) {
        std::cout << "ERROR: Database not enabled!" << std::endl;
        return 1;
    }
    
    std::cout << "Database enabled successfully" << std::endl;
    
    // Create test save data
    SaveData testSaveData;
    testSaveData.playerLevel = 1;
    testSaveData.currentXP = 0;
    testSaveData.maxXP = 100;
    testSaveData.health = 100;
    testSaveData.maxHealth = 100;
    testSaveData.x = 100.0f;
    testSaveData.y = 100.0f;
    testSaveData.currentLevelPath = "test.json";
    testSaveData.saveTime = "2025-09-25 18:30:00";
    testSaveData.totalXP = 0;
    testSaveData.coins = 0;
    testSaveData.playTime = 0;
    testSaveData.enemiesKilled = 0;
    testSaveData.deaths = 0;
    
    std::cout << "Creating temporary player..." << std::endl;
    if (saveManager.createTemporaryPlayer(testSaveData)) {
        std::cout << "✓ Temporary player created successfully" << std::endl;
    } else {
        std::cout << "✗ Failed to create temporary player" << std::endl;
        return 1;
    }
    
    std::cout << "Checking if player is temporary..." << std::endl;
    if (saveManager.isCurrentPlayerTemporary()) {
        std::cout << "✓ Player is correctly marked as temporary" << std::endl;
    } else {
        std::cout << "✗ Player is not marked as temporary" << std::endl;
        return 1;
    }
    
    std::cout << "Making player permanent..." << std::endl;
    if (saveManager.makeCurrentPlayerPermanent()) {
        std::cout << "✓ Player made permanent successfully" << std::endl;
    } else {
        std::cout << "✗ Failed to make player permanent" << std::endl;
        return 1;
    }
    
    std::cout << "Checking if player is now permanent..." << std::endl;
    if (!saveManager.isCurrentPlayerTemporary()) {
        std::cout << "✓ Player is correctly marked as permanent" << std::endl;
    } else {
        std::cout << "✗ Player is still marked as temporary" << std::endl;
        return 1;
    }
    
    std::cout << "All tests passed! Temporary player system is working correctly." << std::endl;
    return 0;
}
