#include "game/GameStateManager.h"
#include <spdlog/spdlog.h>

GameStateManager::GameStateManager() 
    : currentState_(GameState::MENU)
    , selectedMenuOption_(0)
    , selectedPauseButton_(0)
    , selectedSaveSlot_(0)
    , loadSlotFromMainMenu_(false)
    , gameInitialized_(false)
    , deathScreenInitialized_(false) {
}

void GameStateManager::setState(GameState newState) {
    if (currentState_ != newState) {
        spdlog::info("Game state changed from {} to {}", 
                    static_cast<int>(currentState_), 
                    static_cast<int>(newState));
        currentState_ = newState;
    }
}

void GameStateManager::resetMenuSelection() {
    selectedMenuOption_ = 0;
}

void GameStateManager::resetPauseSelection() {
    selectedPauseButton_ = 0;
}

void GameStateManager::resetSaveSlotSelection() {
    selectedSaveSlot_ = 0;
}
