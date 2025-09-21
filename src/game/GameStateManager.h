#pragma once

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    DEATH,
    SAVE_SLOT_SELECTION,
    LOAD_SLOT_SELECTION
};

class GameStateManager {
public:
    GameStateManager();
    ~GameStateManager() = default;
    
    // State management
    GameState getCurrentState() const { return currentState_; }
    void setState(GameState newState);
    
    // State-specific variables
    int getSelectedMenuOption() const { return selectedMenuOption_; }
    void setSelectedMenuOption(int option) { selectedMenuOption_ = option; }
    
    int getSelectedPauseButton() const { return selectedPauseButton_; }
    void setSelectedPauseButton(int button) { selectedPauseButton_ = button; }
    
    int getSelectedSaveSlot() const { return selectedSaveSlot_; }
    void setSelectedSaveSlot(int slot) { selectedSaveSlot_ = slot; }
    
    bool isLoadSlotFromMainMenu() const { return loadSlotFromMainMenu_; }
    void setLoadSlotFromMainMenu(bool fromMain) { loadSlotFromMainMenu_ = fromMain; }
    
    // Game initialization flags
    bool isGameInitialized() const { return gameInitialized_; }
    void setGameInitialized(bool initialized) { gameInitialized_ = initialized; }
    
    bool isDeathScreenInitialized() const { return deathScreenInitialized_; }
    void setDeathScreenInitialized(bool initialized) { deathScreenInitialized_ = initialized; }
    
    // Reset functions
    void resetMenuSelection();
    void resetPauseSelection();
    void resetSaveSlotSelection();
    
private:
    GameState currentState_;
    
    // Menu state variables
    int selectedMenuOption_;
    
    // Pause state variables
    int selectedPauseButton_;
    
    // Save/Load state variables
    int selectedSaveSlot_;
    bool loadSlotFromMainMenu_;
    
    // Game initialization flags
    bool gameInitialized_;
    bool deathScreenInitialized_;
};
