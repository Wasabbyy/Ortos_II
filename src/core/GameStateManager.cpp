#include "core/GameStateManager.h"
#include <spdlog/spdlog.h>

CoreGameStateManager::CoreGameStateManager() 
    : gameplayManager(nullptr), saveManager(nullptr), audioManager(nullptr), uiAudioManager(nullptr), window(nullptr),
      currentState(GameState::MENU), gameInitialized(false), hasSaveFile(false),
      selectedMenuOption(0), previousSelectedMenuOption(-1),
      selectedPauseButton(0), previousSelectedPauseButton(-1),
      selectedSaveSlot(0), loadSlotFromMainMenu(false),
      selectedDeathButton(0), previousSelectedDeathButton(-1),
      mouseX(0.0), mouseY(0.0), respawnButtonHovered(false), exitButtonHovered(false),
      previousRespawnButtonHovered(false), previousExitButtonHovered(false),
      introMusicStarted(false), backgroundMusicStarted(false),
      keyUpPressed(false), keyDownPressed(false), keyEnterPressed(false), keyEscPressed(false),
      mouseLeftPressed(false), hoverSoundPlayed(false), clickSoundPlayed(false) {
}

CoreGameStateManager::~CoreGameStateManager() {
    cleanup();
}

void CoreGameStateManager::initialize(GameplayManager* gameplayManager, 
                                 EnhancedSaveManager* saveManager,
                                 AudioManager* audioManager,
                                 UIAudioManager* uiAudioManager,
                                 GLFWwindow* window,
                                 const std::string& assetPath) {
    this->gameplayManager = gameplayManager;
    this->saveManager = saveManager;
    this->audioManager = audioManager;
    this->uiAudioManager = uiAudioManager;
    this->window = window;
    this->assetPath = assetPath;
    
    // Initialize save slots and check for existing saves
    updateSaveSlots();
    hasSaveFile = saveManager->hasAnySave();
    
    // Start intro music
    startIntroMusic();
    
    spdlog::info("GameStateManager initialized");
}

void CoreGameStateManager::update(float deltaTime, int windowWidth, int windowHeight) {
    updateInputStates();
    
    switch (currentState) {
        case GameState::MENU:
            handleMenuState(windowWidth, windowHeight);
            break;
        case GameState::PLAYING:
            handlePlayingState(deltaTime, windowWidth, windowHeight);
            break;
        case GameState::PAUSED:
            handlePausedState(windowWidth, windowHeight);
            break;
        case GameState::DEATH:
            handleDeathState(windowWidth, windowHeight);
            break;
        case GameState::SAVE_SLOT_SELECTION:
            handleSaveSlotSelectionState(windowWidth, windowHeight);
            break;
        case GameState::LOAD_SLOT_SELECTION:
            handleLoadSlotSelectionState(windowWidth, windowHeight);
            break;
    }
}

void CoreGameStateManager::draw(int windowWidth, int windowHeight) {
    switch (currentState) {
        case GameState::MENU:
            UI::drawMainMenu(windowWidth, windowHeight, selectedMenuOption, hasSaveFile);
            break;
        case GameState::PLAYING:
            // Drawing is handled in handlePlayingState
            break;
        case GameState::PAUSED:
            // Draw game in background (paused state)
            if (gameplayManager->isGameInitialized()) {
                gameplayManager->drawPaused(windowWidth, windowHeight);
            }
            // Draw pause menu overlay
            UI::drawPauseScreen(windowWidth, windowHeight, selectedPauseButton);
            break;
        case GameState::DEATH:
            UI::drawDeathScreen(windowWidth, windowHeight, respawnButtonHovered, exitButtonHovered, selectedDeathButton);
            break;
        case GameState::SAVE_SLOT_SELECTION:
            // Draw game in background (paused)
            if (gameplayManager->isGameInitialized()) {
                gameplayManager->drawPaused(windowWidth, windowHeight);
            }
            // Draw save slot selection menu
            UI::drawSaveSlotMenu(windowWidth, windowHeight, selectedSaveSlot, saveSlotInfo);
            break;
        case GameState::LOAD_SLOT_SELECTION:
            // Draw game in background (paused)
            if (gameplayManager->isGameInitialized()) {
                gameplayManager->drawPaused(windowWidth, windowHeight);
            }
            // Draw load slot selection menu
            UI::drawLoadSlotMenu(windowWidth, windowHeight, selectedSaveSlot, saveSlotInfo);
            break;
    }
}

void CoreGameStateManager::cleanup() {
    stopMusic();
    spdlog::info("GameStateManager cleaned up");
}

void CoreGameStateManager::handleMenuState(int windowWidth, int windowHeight) {
    // Update save slots and check if any save files exist
    updateSaveSlots();
    hasSaveFile = saveManager->hasAnySave();
    
    // Start intro music if not already started
    if (!introMusicStarted) {
        startIntroMusic();
    }
    
    handleMenuInput();
}

void CoreGameStateManager::handlePlayingState(float deltaTime, int windowWidth, int windowHeight) {
    // Initialize game if not already done
    if (!gameplayManager->isGameInitialized()) {
        gameplayManager->startNewGame();
        gameInitialized = true;
        spdlog::info("Game initialized successfully");
        
        // Load sound effects (only load what's available)
        if (!audioManager->loadSound("intro", assetPath + "assets/sounds/intro.wav")) {
            spdlog::warn("Failed to load intro sound");
        }
    }

    // Update and draw gameplay
    gameplayManager->update(deltaTime, window, windowWidth, windowHeight);
    gameplayManager->draw(windowWidth, windowHeight);

    // Check for ESC key to pause game
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !keyEscPressed) {
        selectedPauseButton = 0;
        previousSelectedPauseButton = -1;
        resetInputStates();
        currentState = GameState::PAUSED;
        keyEscPressed = true;
        spdlog::info("Game paused");
        // Update player stats in database when pausing
        gameplayManager->updatePlayerStatsInDatabase();
    } else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        keyEscPressed = false;
    }
    
    // Check if player has died
    if (!gameplayManager->isPlayerAlive()) {
        // Stop background music when player dies
        stopMusic();
        resetMusicState();
        selectedDeathButton = 0;
        previousSelectedDeathButton = -1;
        previousRespawnButtonHovered = false;
        previousExitButtonHovered = false;
        resetInputStates();
        currentState = GameState::DEATH;
        spdlog::info("Player has died, showing death screen");
    }
}

void CoreGameStateManager::handlePausedState(int windowWidth, int windowHeight) {
    handlePauseInput();
}

void CoreGameStateManager::handleDeathState(int windowWidth, int windowHeight) {
    // Handle mouse input for death screen
    glfwGetCursorPos(window, &mouseX, &mouseY);
    float buttonWidth = 260.0f;
    float buttonHeight = 60.0f;
    float buttonX = windowWidth / 2.0f - buttonWidth / 2.0f - 45.0f;
    float respawnButtonY = windowHeight * 0.5f;
    float exitButtonY = windowHeight * 0.35f;

    respawnButtonHovered = isMouseOverButton(mouseX, mouseY, buttonX, respawnButtonY, buttonWidth, buttonHeight);
    exitButtonHovered = isMouseOverButton(mouseX, mouseY, buttonX, exitButtonY, buttonWidth, buttonHeight);

    handleDeathInput();
}

void CoreGameStateManager::handleSaveSlotSelectionState(int windowWidth, int windowHeight) {
    handleSaveSlotInput();
}

void CoreGameStateManager::handleLoadSlotSelectionState(int windowWidth, int windowHeight) {
    handleLoadSlotInput();
}

void CoreGameStateManager::updateInputStates() {
    // This method updates input states for debouncing
    // Most of the actual input handling is done in the specific state handlers
}

void CoreGameStateManager::resetInputStates() {
    keyUpPressed = false;
    keyDownPressed = false;
    keyEnterPressed = false;
    keyEscPressed = false;
    mouseLeftPressed = false;
    hoverSoundPlayed = false;
    clickSoundPlayed = false;
}

void CoreGameStateManager::handleMenuInput() {
    int menuOptions = hasSaveFile ? 3 : 2; // Start Game, Load Game, Exit Game (if save exists) | Start Game, Exit Game (no save)
    
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !keyUpPressed) {
        selectedMenuOption = (selectedMenuOption - 1 + menuOptions) % menuOptions;
        keyUpPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
        keyUpPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !keyDownPressed) {
        selectedMenuOption = (selectedMenuOption + 1) % menuOptions;
        keyDownPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        keyDownPressed = false;
    }
    
    // Play hover sound when selection changes (with debouncing)
    playHoverSound(selectedMenuOption, previousSelectedMenuOption);
    
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed) {
        playClickSound();
        
        if (hasSaveFile) {
            // Menu with save file: Start Game, Load Game, Exit Game
            if (selectedMenuOption == 0) {
                // Start new game - always create fresh game
                spdlog::info("Starting fresh new game");
                gameInitialized = false; // Force re-initialization
                gameplayManager->resetGame(); // Reset the gameplay manager state
                transitionToPlaying();
            } else if (selectedMenuOption == 1) {
                // Load game - go to load slot selection
                transitionToLoadSlotSelection();
                spdlog::info("Entering load slot selection from main menu");
            } else if (selectedMenuOption == 2) {
                // Exit game
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        } else {
            // Menu without save file: Start Game, Exit Game
            if (selectedMenuOption == 0) {
                // Start new game - reset game state
                spdlog::info("Starting fresh new game");
                gameInitialized = false; // Force re-initialization
                gameplayManager->resetGame(); // Reset the gameplay manager state
                transitionToPlaying();
            } else if (selectedMenuOption == 1) {
                // Exit game
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
        
        keyEnterPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        keyEnterPressed = false;
    }
}

void CoreGameStateManager::handlePauseInput() {
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !keyUpPressed) {
        selectedPauseButton = (selectedPauseButton - 1 + 4) % 4; // 4 options: Resume, Save Game, Back to Menu, Exit Game
        keyUpPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
        keyUpPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !keyDownPressed) {
        selectedPauseButton = (selectedPauseButton + 1) % 4;
        keyDownPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        keyDownPressed = false;
    }
    
    // Play hover sound when selection changes (with debouncing)
    playHoverSound(selectedPauseButton, previousSelectedPauseButton);
    
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed) {
        playClickSound();
        
        if (selectedPauseButton == 0) {
            // Resume game
            currentState = GameState::PLAYING;
            spdlog::info("Resuming game");
        } else if (selectedPauseButton == 1) {
            // Save game - go to save slot selection
            transitionToSaveSlotSelection();
            spdlog::info("Entering save slot selection");
        } else if (selectedPauseButton == 2) {
            // Back to main menu
            transitionToMenu();
            spdlog::info("Returning to main menu from pause");
        } else if (selectedPauseButton == 3) {
            // Exit game
            spdlog::info("Exiting game from pause menu");
            // Update player stats in database before exiting
            gameplayManager->updatePlayerStatsInDatabase();
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        keyEnterPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        keyEnterPressed = false;
    }
    
    // Also allow ESC to resume (alternative to Resume button)
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !keyEscPressed) {
        currentState = GameState::PLAYING;
        keyEscPressed = true;
        spdlog::info("Resuming game with ESC key");
    } else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        keyEscPressed = false;
    }
}

void CoreGameStateManager::handleDeathInput() {
    // Keyboard navigation (identical to main menu)
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !keyUpPressed) {
        selectedDeathButton = (selectedDeathButton - 1 + 2) % 2;
        keyUpPressed = true;
        spdlog::debug("Death screen: Up arrow pressed, selected button: {}", selectedDeathButton);
    } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
        keyUpPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !keyDownPressed) {
        selectedDeathButton = (selectedDeathButton + 1) % 2;
        keyDownPressed = true;
        spdlog::debug("Death screen: Down arrow pressed, selected button: {}", selectedDeathButton);
    } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        keyDownPressed = false;
    }

    // Play hover sound when selection changes (with debouncing)
    playHoverSound(selectedDeathButton, previousSelectedDeathButton);

    // Mouse click (still works for clicking)
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mouseLeftPressed) {
        if (respawnButtonHovered) {
            // Play click sound before respawning
            playClickSound();
            // Reset game state
            spdlog::info("Respawn button clicked, restarting game");
            gameplayManager->resetGame();
            gameInitialized = false;
            transitionToPlaying();
        } else if (exitButtonHovered) {
            // Play click sound before exiting
            playClickSound();
            spdlog::info("Exit button clicked, exiting game");
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        mouseLeftPressed = true;
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        mouseLeftPressed = false;
    }

    // Keyboard Enter
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed) {
        if (selectedDeathButton == 0) {
            // Play click sound before respawning
            playClickSound();
            spdlog::info("Enter pressed on Respawn, restarting game");
            gameplayManager->resetGame();
            gameInitialized = false;
            transitionToPlaying();
        } else if (selectedDeathButton == 1) {
            // Play click sound before exiting
            playClickSound();
            spdlog::info("Enter pressed on Exit, exiting game");
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        keyEnterPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        keyEnterPressed = false;
    }
}

void CoreGameStateManager::handleSaveSlotInput() {
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !keyUpPressed) {
        selectedSaveSlot = (selectedSaveSlot - 1 + 4) % 4; // 3 slots + back button
        keyUpPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
        keyUpPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !keyDownPressed) {
        selectedSaveSlot = (selectedSaveSlot + 1) % 4;
        keyDownPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        keyDownPressed = false;
    }
    
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed) {
        if (selectedSaveSlot < 3) {
            // Save to selected slot
            if (gameplayManager->isGameInitialized()) {
                SaveData saveData = gameplayManager->createSaveData();
                
                if (saveManager->saveGame(saveData, selectedSaveSlot)) {
                    saveManager->updateSaveSlots();
                    hasSaveFile = true;
                    spdlog::info("Game saved to slot {}", selectedSaveSlot + 1);
                } else {
                    spdlog::error("Failed to save game to slot {}", selectedSaveSlot + 1);
                }
            }
            currentState = GameState::PAUSED;
        } else {
            // Back button
            currentState = GameState::PAUSED;
        }
        keyEnterPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        keyEnterPressed = false;
    }
}

void CoreGameStateManager::handleLoadSlotInput() {
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !keyUpPressed) {
        selectedSaveSlot = (selectedSaveSlot - 1 + 4) % 4; // 3 slots + back button
        keyUpPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
        keyUpPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !keyDownPressed) {
        selectedSaveSlot = (selectedSaveSlot + 1) % 4;
        keyDownPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        keyDownPressed = false;
    }
    
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed) {
        if (selectedSaveSlot < 3 && saveManager->getSaveSlot(selectedSaveSlot).hasSave()) {
            // Load from selected slot
            SaveData saveData;
            if (saveManager->loadGame(saveData, selectedSaveSlot)) {
                // Load the game state using GameplayManager
                gameplayManager->loadGame(saveData, assetPath);
                
                spdlog::info("Game loaded from slot {}", selectedSaveSlot + 1);
                // Go to playing state if loaded from main menu, otherwise back to pause
                if (loadSlotFromMainMenu) {
                    currentState = GameState::PLAYING;
                } else {
                    currentState = GameState::PAUSED;
                }
            } else {
                spdlog::error("Failed to load game from slot {}", selectedSaveSlot + 1);
                // Go back to appropriate menu based on where we came from
                if (loadSlotFromMainMenu) {
                    currentState = GameState::MENU;
                } else {
                    currentState = GameState::PAUSED;
                }
            }
        } else if (selectedSaveSlot == 3) {
            // Back button - go back to appropriate menu
            if (loadSlotFromMainMenu) {
                currentState = GameState::MENU;
            } else {
                currentState = GameState::PAUSED;
            }
        }
        keyEnterPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        keyEnterPressed = false;
    }
}

void CoreGameStateManager::startIntroMusic() {
    audioManager->playMusic("intro", true); // Loop the intro music
    introMusicStarted = true;
    spdlog::info("Started intro music");
}

void CoreGameStateManager::startGameplayMusic() {
    // Stop intro music when starting game
    audioManager->stopMusic();
    introMusicStarted = false;
    // Set lower volume for background music during gameplay
    audioManager->setMusicVolume(0.4f); // Reduced from default 1.0 to 0.4
    // Start background music for gameplay
    audioManager->playMusic("background", true); // Loop the background music
    backgroundMusicStarted = true;
    spdlog::info("Started background music for gameplay at reduced volume (0.4)");
}

void CoreGameStateManager::stopMusic() {
    audioManager->stopMusic();
    backgroundMusicStarted = false;
    spdlog::info("Stopped background music due to player death");
}

void CoreGameStateManager::resetMusicState() {
    // Reset music volume to normal for intro music
    audioManager->setMusicVolume(1.0f);
}

void CoreGameStateManager::transitionToMenu() {
    // Stop background music and reset introMusicStarted flag
    audioManager->stopMusic();
    introMusicStarted = false;
    backgroundMusicStarted = false;
    // Reset music volume to normal for intro music
    audioManager->setMusicVolume(1.0f);
    // Reset menu hover tracking
    previousSelectedMenuOption = -1;
    currentState = GameState::MENU;
}

void CoreGameStateManager::transitionToPlaying() {
    // Start background music when entering game
    startGameplayMusic();
    currentState = GameState::PLAYING;
}

void CoreGameStateManager::transitionToPaused() {
    currentState = GameState::PAUSED;
}

void CoreGameStateManager::transitionToDeath() {
    currentState = GameState::DEATH;
}

void CoreGameStateManager::transitionToSaveSlotSelection() {
    saveManager->updateSaveSlots();
    saveSlotInfo = saveManager->getSaveSlotInfo();
    selectedSaveSlot = 0;
    currentState = GameState::SAVE_SLOT_SELECTION;
}

void CoreGameStateManager::transitionToLoadSlotSelection() {
    saveManager->updateSaveSlots();
    saveSlotInfo = saveManager->getSaveSlotInfo();
    selectedSaveSlot = 0;
    loadSlotFromMainMenu = true;
    currentState = GameState::LOAD_SLOT_SELECTION;
}

void CoreGameStateManager::updateSaveSlots() {
    saveManager->updateSaveSlots();
}

void CoreGameStateManager::playHoverSound(int currentSelection, int& previousSelection) {
    if (currentSelection != previousSelection) {
        if (!hoverSoundPlayed) {
            uiAudioManager->playButtonHoverSound();
            hoverSoundPlayed = true;
        }
        previousSelection = currentSelection;
    } else {
        hoverSoundPlayed = false; // Reset when not changing
    }
}

void CoreGameStateManager::playClickSound() {
    uiAudioManager->playButtonClickSound();
}

bool CoreGameStateManager::isMouseOverButton(double mouseX, double mouseY, float buttonX, float buttonY, float buttonWidth, float buttonHeight) {
    return UI::isMouseOverButton(mouseX, mouseY, buttonX, buttonY, buttonWidth, buttonHeight);
}
