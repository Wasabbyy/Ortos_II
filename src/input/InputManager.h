#pragma once

#include <GLFW/glfw3.h>
#include "game/GameStateManager.h"

class InputManager {
public:
    InputManager(GLFWwindow* window);
    ~InputManager() = default;
    
    // Input processing
    void processInput(GameStateManager& stateManager);
    
    // Debouncing
    void resetDebouncing();
    
    // Sound debouncing
    bool canPlayHoverSound() const { return !hoverSoundPlayed_; }
    void setHoverSoundPlayed(bool played) { hoverSoundPlayed_ = played; }
    bool canPlayClickSound() const { return !clickSoundPlayed_; }
    void setClickSoundPlayed(bool played) { clickSoundPlayed_ = played; }
    
private:
    GLFWwindow* window_;
    
    // Input debouncing
    bool keyUpPressed_;
    bool keyDownPressed_;
    bool keyEnterPressed_;
    bool keyEscPressed_;
    
    // Sound debouncing
    bool hoverSoundPlayed_;
    bool clickSoundPlayed_;
    
    // Previous states for hover sound detection
    int previousSelectedMenuOption_;
    int previousSelectedPauseButton_;
    int previousSelectedSaveSlot_;
    
    // Helper functions
    void processMenuInput(GameStateManager& stateManager, int menuOptions);
    void processPauseInput(GameStateManager& stateManager);
    void processSaveSlotInput(GameStateManager& stateManager);
    void processLoadSlotInput(GameStateManager& stateManager);
    void processDeathScreenInput(GameStateManager& stateManager);
    
    void handleMenuNavigation(int& selectedOption, int maxOptions);
    void handlePauseNavigation(int& selectedButton, int maxButtons);
    void handleSaveSlotNavigation(int& selectedSlot, int maxSlots);
};
