#include "game/GameLoopManager.h"
#include "ui/UI.h"
#include <spdlog/spdlog.h>

GameLoopManager::GameLoopManager(GLFWwindow* window, AudioManager& audioManager, UIAudioManager& uiAudioManager)
    : window_(window)
    , audioManager_(audioManager)
    , uiAudioManager_(uiAudioManager)
    , inputManager_(window)
    , player_(nullptr)
    , tilemap_(nullptr)
    , currentLevelPath_("assets/levels/level1.json")
    , levelTransitionCooldown_(0.0f)
    , hasSaveFile_(false) {
}

GameLoopManager::~GameLoopManager() {
    cleanupGame();
}

void GameLoopManager::run() {
    while (!glfwWindowShouldClose(window_)) {
        // Process input
        inputManager_.processInput(stateManager_);
        
        // Handle game logic based on current state
        switch (stateManager_.getCurrentState()) {
            case GameState::MENU:
                handleMenuLogic();
                break;
            case GameState::PLAYING:
                updateGameplay();
                break;
            case GameState::PAUSED:
                handlePauseLogic();
                break;
            case GameState::DEATH:
                handleDeathLogic();
                break;
            case GameState::SAVE_SLOT_SELECTION:
                handleSaveSlotLogic();
                break;
            case GameState::LOAD_SLOT_SELECTION:
                handleLoadSlotLogic();
                break;
        }
        
        // Render
        render();
        
        // Swap buffers and poll events
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
}

bool GameLoopManager::initializeGame() {
    // Initialize game objects
    player_ = new Player();
    tilemap_ = new Tilemap();
    
    if (!tilemap_->loadFromJSON(currentLevelPath_)) {
        spdlog::error("Failed to load initial level: {}", currentLevelPath_);
        return false;
    }
    
    // Create initial enemies
    enemies_.push_back(new Enemy(EnemyType::FlyingEye));
    enemies_.push_back(new Enemy(EnemyType::Shroom));
    
    stateManager_.setGameInitialized(true);
    return true;
}

void GameLoopManager::cleanupGame() {
    delete player_;
    player_ = nullptr;
    
    for (auto* enemy : enemies_) {
        delete enemy;
    }
    enemies_.clear();
    
    delete tilemap_;
    tilemap_ = nullptr;
}

void GameLoopManager::render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    int windowWidth, windowHeight;
    glfwGetWindowSize(window_, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    switch (stateManager_.getCurrentState()) {
        case GameState::MENU:
            renderMenu();
            break;
        case GameState::PLAYING:
        case GameState::PAUSED:
            renderGameplay();
            if (stateManager_.getCurrentState() == GameState::PAUSED) {
                renderPauseScreen();
            }
            break;
        case GameState::DEATH:
            renderDeathScreen();
            break;
        case GameState::SAVE_SLOT_SELECTION:
            renderSaveSlotMenu();
            break;
        case GameState::LOAD_SLOT_SELECTION:
            renderLoadSlotMenu();
            break;
    }
}

void GameLoopManager::renderMenu() {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window_, &windowWidth, &windowHeight);
    
    // Update save file status
    saveManager_.updateSaveSlots();
    hasSaveFile_ = saveManager_.hasAnySaveFile();
    
    UI::drawMainMenu(windowWidth, windowHeight, stateManager_.getSelectedMenuOption(), hasSaveFile_);
}

void GameLoopManager::renderPauseScreen() {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window_, &windowWidth, &windowHeight);
    UI::drawPauseScreen(windowWidth, windowHeight, stateManager_.getSelectedPauseButton());
}

void GameLoopManager::renderDeathScreen() {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window_, &windowWidth, &windowHeight);
    UI::drawDeathScreen(windowWidth, windowHeight, stateManager_.getSelectedMenuOption());
}

void GameLoopManager::renderSaveSlotMenu() {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window_, &windowWidth, &windowHeight);
    auto saveSlotInfo = generateSaveSlotInfo();
    UI::drawSaveSlotMenu(windowWidth, windowHeight, stateManager_.getSelectedSaveSlot(), saveSlotInfo);
}

void GameLoopManager::renderLoadSlotMenu() {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window_, &windowWidth, &windowHeight);
    auto loadSlotInfo = generateLoadSlotInfo();
    UI::drawLoadSlotMenu(windowWidth, windowHeight, stateManager_.getSelectedSaveSlot(), loadSlotInfo);
}

void GameLoopManager::renderGameplay() {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window_, &windowWidth, &windowHeight);
    
    if (stateManager_.isGameInitialized()) {
        // Set up 2D projection for gameplay
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Draw game objects
        if (tilemap_) tilemap_->draw();
        if (player_) player_->draw();
        
        for (auto* enemy : enemies_) {
            if (enemy) enemy->draw();
        }
        
        for (const auto& projectile : playerProjectiles_) {
            projectile.draw();
        }
        
        for (const auto& projectile : enemyProjectiles_) {
            projectile.draw();
        }
        
        for (const auto& bloodEffect : bloodEffects_) {
            bloodEffect.draw();
        }
        
        // Draw UI elements
        UI::drawHUD(windowWidth, windowHeight, player_);
    }
}

void GameLoopManager::updateGameplay() {
    if (!stateManager_.isGameInitialized()) return;
    
    // Update game objects
    if (player_) {
        player_->update(inputHandler_);
    }
    
    for (auto* enemy : enemies_) {
        if (enemy) {
            enemy->update(player_, enemyProjectiles_);
        }
    }
    
    for (auto& projectile : playerProjectiles_) {
        projectile.update();
    }
    
    for (auto& projectile : enemyProjectiles_) {
        projectile.update();
    }
    
    for (auto it = bloodEffects_.begin(); it != bloodEffects_.end();) {
        it->update();
        if (!it->isActive()) {
            it = bloodEffects_.erase(it);
        } else {
            ++it;
        }
    }
    
    // Handle collisions and game logic
    // (This would contain the collision detection and game logic from the original main.cpp)
}

void GameLoopManager::handleMenuLogic() {
    // Handle menu option selection
    if (inputManager_.canPlayClickSound()) {
        int selectedOption = stateManager_.getSelectedMenuOption();
        
        if (hasSaveFile_) {
            // Menu with save file: Start Game, Load Game, Exit Game
            if (selectedOption == 0) {
                startNewGame();
            } else if (selectedOption == 1) {
                stateManager_.setLoadSlotFromMainMenu(true);
                stateManager_.setState(GameState::LOAD_SLOT_SELECTION);
                stateManager_.resetSaveSlotSelection();
            } else if (selectedOption == 2) {
                glfwSetWindowShouldClose(window_, GLFW_TRUE);
            }
        } else {
            // Menu without save file: Start Game, Exit Game
            if (selectedOption == 0) {
                startNewGame();
            } else if (selectedOption == 1) {
                glfwSetWindowShouldClose(window_, GLFW_TRUE);
            }
        }
        
        inputManager_.setClickSoundPlayed(true);
    }
}

void GameLoopManager::handlePauseLogic() {
    if (inputManager_.canPlayClickSound()) {
        int selectedButton = stateManager_.getSelectedPauseButton();
        
        if (selectedButton == 0) {
            // Resume
            stateManager_.setState(GameState::PLAYING);
        } else if (selectedButton == 1) {
            // Save Game
            stateManager_.setState(GameState::SAVE_SLOT_SELECTION);
            stateManager_.resetSaveSlotSelection();
        } else if (selectedButton == 2) {
            // Back to Menu
            resetGameState();
            stateManager_.setState(GameState::MENU);
            stateManager_.resetMenuSelection();
            audioManager_.stopMusic();
            audioManager_.playMusic("intro");
        } else if (selectedButton == 3) {
            // Exit Game
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
        }
        
        inputManager_.setClickSoundPlayed(true);
    }
}

void GameLoopManager::handleDeathLogic() {
    if (inputManager_.canPlayClickSound()) {
        int selectedOption = stateManager_.getSelectedMenuOption();
        
        if (selectedOption == 0) {
            // Restart Game
            startNewGame();
        } else if (selectedOption == 1) {
            // Back to Menu
            resetGameState();
            stateManager_.setState(GameState::MENU);
            stateManager_.resetMenuSelection();
            audioManager_.stopMusic();
            audioManager_.playMusic("intro");
        }
        
        inputManager_.setClickSoundPlayed(true);
    }
}

void GameLoopManager::handleSaveSlotLogic() {
    if (inputManager_.canPlayClickSound()) {
        int selectedSlot = stateManager_.getSelectedSaveSlot();
        
        // Create save data
        SaveData saveData = saveManager_.createSaveData(player_, enemies_, playerProjectiles_, enemyProjectiles_, currentLevelPath_, levelTransitionCooldown_);
        
        // Save to selected slot
        const auto& saveSlots = saveManager_.getSaveSlots();
        if (saveManager_.saveGame(saveData, saveSlots[selectedSlot].filename)) {
            spdlog::info("Game saved to slot {}", selectedSlot + 1);
        }
        
        // Return to pause menu
        stateManager_.setState(GameState::PAUSED);
        stateManager_.resetSaveSlotSelection();
        
        inputManager_.setClickSoundPlayed(true);
    }
}

void GameLoopManager::handleLoadSlotLogic() {
    if (inputManager_.canPlayClickSound()) {
        int selectedSlot = stateManager_.getSelectedSaveSlot();
        const auto& saveSlots = saveManager_.getSaveSlots();
        
        if (saveSlots[selectedSlot].exists) {
            SaveData saveData;
            if (saveManager_.loadGame(saveData, saveSlots[selectedSlot].filename)) {
                // Load the game state
                saveManager_.loadGameState(saveData, player_, enemies_, playerProjectiles_, enemyProjectiles_, tilemap_, currentLevelPath_, levelTransitionCooldown_);
                
                // Transition to appropriate state
                if (stateManager_.isLoadSlotFromMainMenu()) {
                    stateManager_.setState(GameState::PLAYING);
                    audioManager_.stopMusic();
                    audioManager_.playMusic("background");
                } else {
                    stateManager_.setState(GameState::PAUSED);
                }
                
                stateManager_.setGameInitialized(true);
                spdlog::info("Game loaded from slot {}", selectedSlot + 1);
            }
        }
        
        stateManager_.resetSaveSlotSelection();
        stateManager_.setLoadSlotFromMainMenu(false);
        
        inputManager_.setClickSoundPlayed(true);
    }
}

void GameLoopManager::startNewGame() {
    spdlog::info("Starting new game");
    resetGameState();
    
    if (!initializeGame()) {
        spdlog::error("Failed to initialize new game");
        return;
    }
    
    stateManager_.setState(GameState::PLAYING);
    audioManager_.stopMusic();
    audioManager_.playMusic("background");
}

void GameLoopManager::resetGameState() {
    cleanupGame();
    stateManager_.setGameInitialized(false);
    stateManager_.setDeathScreenInitialized(false);
    currentLevelPath_ = "assets/levels/level1.json";
    levelTransitionCooldown_ = 0.0f;
    playerProjectiles_.clear();
    enemyProjectiles_.clear();
    bloodEffects_.clear();
}

std::vector<std::string> GameLoopManager::generateSaveSlotInfo() {
    std::vector<std::string> info;
    const auto& saveSlots = saveManager_.getSaveSlots();
    
    for (int i = 0; i < SaveManager::MAX_SAVE_SLOTS; i++) {
        if (saveSlots[i].exists) {
            info.push_back("Slot " + std::to_string(i + 1) + " - " + saveSlots[i].saveTime.substr(0, 10));
        } else {
            info.push_back("Slot " + std::to_string(i + 1) + " - Empty");
        }
    }
    
    return info;
}

std::vector<std::string> GameLoopManager::generateLoadSlotInfo() {
    return generateSaveSlotInfo(); // Same logic for both save and load menus
}
