#include "input/InputManager.h"
#include <spdlog/spdlog.h>

InputManager::InputManager(GLFWwindow* window) 
    : window_(window)
    , keyUpPressed_(false)
    , keyDownPressed_(false)
    , keyEnterPressed_(false)
    , keyEscPressed_(false)
    , hoverSoundPlayed_(false)
    , clickSoundPlayed_(false)
    , previousSelectedMenuOption_(0)
    , previousSelectedPauseButton_(0)
    , previousSelectedSaveSlot_(0) {
}

void InputManager::processInput(GameStateManager& stateManager) {
    GameState currentState = stateManager.getCurrentState();
    
    switch (currentState) {
        case GameState::MENU: {
            int menuOptions = stateManager.isLoadSlotFromMainMenu() ? 3 : 2;
            processMenuInput(stateManager, menuOptions);
            break;
        }
        case GameState::PAUSED:
            processPauseInput(stateManager);
            break;
        case GameState::SAVE_SLOT_SELECTION:
            processSaveSlotInput(stateManager);
            break;
        case GameState::LOAD_SLOT_SELECTION:
            processLoadSlotInput(stateManager);
            break;
        case GameState::DEATH:
            processDeathScreenInput(stateManager);
            break;
        case GameState::PLAYING:
            // Handle ESC key for pause
            if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS && !keyEscPressed_) {
                keyEscPressed_ = true;
                stateManager.setState(GameState::PAUSED);
                spdlog::info("Game paused");
            } else if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
                keyEscPressed_ = false;
            }
            break;
    }
}

void InputManager::resetDebouncing() {
    keyUpPressed_ = false;
    keyDownPressed_ = false;
    keyEnterPressed_ = false;
    keyEscPressed_ = false;
    hoverSoundPlayed_ = false;
    clickSoundPlayed_ = false;
}

void InputManager::processMenuInput(GameStateManager& stateManager, int menuOptions) {
    // Handle Up/Down navigation
    if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS && !keyUpPressed_) {
        keyUpPressed_ = true;
        stateManager.setSelectedMenuOption((stateManager.getSelectedMenuOption() - 1 + menuOptions) % menuOptions);
    } else if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_RELEASE) {
        keyUpPressed_ = false;
    }
    
    if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS && !keyDownPressed_) {
        keyDownPressed_ = true;
        stateManager.setSelectedMenuOption((stateManager.getSelectedMenuOption() + 1) % menuOptions);
    } else if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        keyDownPressed_ = false;
    }
    
    // Handle Enter key
    if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed_) {
        keyEnterPressed_ = true;
        clickSoundPlayed_ = true;
        spdlog::info("Menu option {} selected", stateManager.getSelectedMenuOption());
    } else if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        keyEnterPressed_ = false;
        clickSoundPlayed_ = false;
    }
    
    // Handle hover sound
    if (stateManager.getSelectedMenuOption() != previousSelectedMenuOption_) {
        if (!hoverSoundPlayed_) {
            hoverSoundPlayed_ = true;
        }
        previousSelectedMenuOption_ = stateManager.getSelectedMenuOption();
    } else {
        hoverSoundPlayed_ = false;
    }
}

void InputManager::processPauseInput(GameStateManager& stateManager) {
    const int pauseOptions = 4; // Resume, Save Game, Back to Menu, Exit Game
    
    // Handle Up/Down navigation
    if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS && !keyUpPressed_) {
        keyUpPressed_ = true;
        stateManager.setSelectedPauseButton((stateManager.getSelectedPauseButton() - 1 + pauseOptions) % pauseOptions);
    } else if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_RELEASE) {
        keyUpPressed_ = false;
    }
    
    if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS && !keyDownPressed_) {
        keyDownPressed_ = true;
        stateManager.setSelectedPauseButton((stateManager.getSelectedPauseButton() + 1) % pauseOptions);
    } else if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        keyDownPressed_ = false;
    }
    
    // Handle Enter key
    if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed_) {
        keyEnterPressed_ = true;
        clickSoundPlayed_ = true;
        spdlog::info("Pause option {} selected", stateManager.getSelectedPauseButton());
    } else if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        keyEnterPressed_ = false;
        clickSoundPlayed_ = false;
    }
    
    // Handle ESC key to resume
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS && !keyEscPressed_) {
        keyEscPressed_ = true;
        stateManager.setState(GameState::PLAYING);
        spdlog::info("Game resumed");
    } else if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        keyEscPressed_ = false;
    }
    
    // Handle hover sound
    if (stateManager.getSelectedPauseButton() != previousSelectedPauseButton_) {
        if (!hoverSoundPlayed_) {
            hoverSoundPlayed_ = true;
        }
        previousSelectedPauseButton_ = stateManager.getSelectedPauseButton();
    } else {
        hoverSoundPlayed_ = false;
    }
}

void InputManager::processSaveSlotInput(GameStateManager& stateManager) {
    const int maxSlots = 3;
    
    // Handle Up/Down navigation
    if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS && !keyUpPressed_) {
        keyUpPressed_ = true;
        stateManager.setSelectedSaveSlot((stateManager.getSelectedSaveSlot() - 1 + maxSlots) % maxSlots);
    } else if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_RELEASE) {
        keyUpPressed_ = false;
    }
    
    if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS && !keyDownPressed_) {
        keyDownPressed_ = true;
        stateManager.setSelectedSaveSlot((stateManager.getSelectedSaveSlot() + 1) % maxSlots);
    } else if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        keyDownPressed_ = false;
    }
    
    // Handle Enter key
    if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed_) {
        keyEnterPressed_ = true;
        clickSoundPlayed_ = true;
        spdlog::info("Save slot {} selected", stateManager.getSelectedSaveSlot() + 1);
    } else if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        keyEnterPressed_ = false;
        clickSoundPlayed_ = false;
    }
    
    // Handle ESC key to go back
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS && !keyEscPressed_) {
        keyEscPressed_ = true;
        stateManager.setState(GameState::PAUSED);
        stateManager.resetSaveSlotSelection();
        spdlog::info("Returned to pause menu from save slot selection");
    } else if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        keyEscPressed_ = false;
    }
    
    // Handle hover sound
    if (stateManager.getSelectedSaveSlot() != previousSelectedSaveSlot_) {
        if (!hoverSoundPlayed_) {
            hoverSoundPlayed_ = true;
        }
        previousSelectedSaveSlot_ = stateManager.getSelectedSaveSlot();
    } else {
        hoverSoundPlayed_ = false;
    }
}

void InputManager::processLoadSlotInput(GameStateManager& stateManager) {
    const int maxSlots = 3;
    
    // Handle Up/Down navigation
    if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS && !keyUpPressed_) {
        keyUpPressed_ = true;
        stateManager.setSelectedSaveSlot((stateManager.getSelectedSaveSlot() - 1 + maxSlots) % maxSlots);
    } else if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_RELEASE) {
        keyUpPressed_ = false;
    }
    
    if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS && !keyDownPressed_) {
        keyDownPressed_ = true;
        stateManager.setSelectedSaveSlot((stateManager.getSelectedSaveSlot() + 1) % maxSlots);
    } else if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        keyDownPressed_ = false;
    }
    
    // Handle Enter key
    if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed_) {
        keyEnterPressed_ = true;
        clickSoundPlayed_ = true;
        spdlog::info("Load slot {} selected", stateManager.getSelectedSaveSlot() + 1);
    } else if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        keyEnterPressed_ = false;
        clickSoundPlayed_ = false;
    }
    
    // Handle ESC key to go back
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS && !keyEscPressed_) {
        keyEscPressed_ = true;
        if (stateManager.isLoadSlotFromMainMenu()) {
            stateManager.setState(GameState::MENU);
        } else {
            stateManager.setState(GameState::PAUSED);
        }
        stateManager.resetSaveSlotSelection();
        spdlog::info("Returned from load slot selection");
    } else if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        keyEscPressed_ = false;
    }
    
    // Handle hover sound
    if (stateManager.getSelectedSaveSlot() != previousSelectedSaveSlot_) {
        if (!hoverSoundPlayed_) {
            hoverSoundPlayed_ = true;
        }
        previousSelectedSaveSlot_ = stateManager.getSelectedSaveSlot();
    } else {
        hoverSoundPlayed_ = false;
    }
}

void InputManager::processDeathScreenInput(GameStateManager& stateManager) {
    const int deathOptions = 2; // Restart Game, Back to Menu
    
    // Handle Up/Down navigation
    if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS && !keyUpPressed_) {
        keyUpPressed_ = true;
        stateManager.setSelectedMenuOption((stateManager.getSelectedMenuOption() - 1 + deathOptions) % deathOptions);
    } else if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_RELEASE) {
        keyUpPressed_ = false;
    }
    
    if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS && !keyDownPressed_) {
        keyDownPressed_ = true;
        stateManager.setSelectedMenuOption((stateManager.getSelectedMenuOption() + 1) % deathOptions);
    } else if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        keyDownPressed_ = false;
    }
    
    // Handle Enter key
    if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed_) {
        keyEnterPressed_ = true;
        clickSoundPlayed_ = true;
        spdlog::info("Death screen option {} selected", stateManager.getSelectedMenuOption());
    } else if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        keyEnterPressed_ = false;
        clickSoundPlayed_ = false;
    }
    
    // Handle hover sound
    if (stateManager.getSelectedMenuOption() != previousSelectedMenuOption_) {
        if (!hoverSoundPlayed_) {
            hoverSoundPlayed_ = true;
        }
        previousSelectedMenuOption_ = stateManager.getSelectedMenuOption();
    } else {
        hoverSoundPlayed_ = false;
    }
}
