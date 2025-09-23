#include <GLFW/glfw3.h>
#include <AL/al.h>
#include "player/Player.h"
#include "enemy/Enemy.h"
#include "projectile/Projectile.h"
#include "effects/BloodEffect.h"
#include "audio/AudioManager.h"
#include "audio/UIAudioManager.h"
#include "input/InputHandler.h"
#include "map/TileMap.h"
#include "ui/UI.h"
#include "save/SaveData.h"
#include "save/SaveSlot.h"
#include "save/SaveManager.h"
#include "save/GameStateManager.h"
#include "collision/CollisionManager.h"
#include "core/GameInitializer.h"
#include "core/GameplayManager.h"
#include <iostream>
#include <stb_image.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <filesystem>
#include <algorithm>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

// Asset path function is now handled by GameInitializer

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    DEATH,
    SAVE_SLOT_SELECTION,
    LOAD_SLOT_SELECTION
};


int main() {
    // Initialize the game using GameInitializer
    GameInitializer initializer;
    if (!initializer.initialize()) {
        spdlog::error("Failed to initialize game");
        return -1;
    }

    // Get initialized components
    GLFWwindow* window = initializer.getWindow();
    AudioManager* audioManager = initializer.getAudioManager();
    UIAudioManager* uiAudioManager = initializer.getUIAudioManager();
    
    // Initialize save manager
    SaveManager saveManager(initializer.getAssetPath("saves/"));
    saveManager.initialize();
    
    // Initialize collision manager
    CollisionManager collisionManager;
    
    // Initialize gameplay manager
    GameplayManager gameplayManager;
    if (!gameplayManager.initialize(initializer.getAssetPath(""), audioManager, uiAudioManager)) {
        spdlog::error("Failed to initialize gameplay manager");
        return -1;
    }

    // Get window dimensions
    int windowWidth = 1920;
    int windowHeight = 1080;

    // Game state management
    GameState currentState = GameState::MENU;
    int selectedMenuOption = 0;
    bool gameInitialized = false;
    // Check if any save slots exist
    bool hasSaveFile = false;
    bool introMusicStarted = false;
    bool backgroundMusicStarted = false;
    
    // Save slot selection variables
    int selectedSaveSlot = 0;
    bool saveSlotMenuInitialized = false;
    bool loadSlotMenuInitialized = false;
    std::vector<std::string> saveSlotInfo;
    bool loadSlotFromMainMenu = false;
    // Level management is now handled by GameplayManager
    
    // Input debouncing
    bool keyUpPressed = false;
    bool keyDownPressed = false;
    bool keyEnterPressed = false;
    bool keyEscPressed = false;
    
    // Sound debouncing
    bool hoverSoundPlayed = false;
    bool clickSoundPlayed = false;
    
    // Mouse input for death screen
    double mouseX = 0.0, mouseY = 0.0;
    bool mouseLeftPressed = false;
    bool respawnButtonHovered = false;
    
    // Button hover tracking for sound effects
    int previousSelectedMenuOption = -1;
    int previousSelectedDeathButton = -1;
    bool previousRespawnButtonHovered = false;
    bool previousExitButtonHovered = false;

    // Game objects are now managed by GameplayManager

    float lastTime = glfwGetTime();

    // --- Add these at the top of main, after other variables ---
    int selectedDeathButton = 0;
    bool deathScreenInitialized = false;
    
    // Pause menu variables
    int selectedPauseButton = 0;
    bool pauseScreenInitialized = false;
    bool previousSelectedPauseButton = -1;

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT);

        if (currentState == GameState::MENU) {
            // Update save slots and check if any save files exist
            saveManager.updateSaveSlots();
            hasSaveFile = saveManager.hasAnySave();
            
            // Start intro music if not already started
            if (!introMusicStarted) {
                audioManager->playMusic("intro", true); // Loop the intro music
                introMusicStarted = true;
                spdlog::info("Started intro music");
            }
            
            // Handle menu input
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
            if (selectedMenuOption != previousSelectedMenuOption) {
                if (!hoverSoundPlayed) {
                uiAudioManager->playButtonHoverSound();
                    hoverSoundPlayed = true;
                }
                previousSelectedMenuOption = selectedMenuOption;
            } else {
                hoverSoundPlayed = false; // Reset when not changing
            }
            
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed) {
                    uiAudioManager->playButtonClickSound();
                
                if (hasSaveFile) {
                    // Menu with save file: Start Game, Continue Game, Load Game, Exit Game
                    if (selectedMenuOption == 0) {
                        // Start new game - reset game state
                        spdlog::info("Starting new game");
                        gameInitialized = false; // Force re-initialization
                        currentState = GameState::PLAYING;
                    } else if (selectedMenuOption == 1) {
                        // Load game - go to load slot selection
                        saveManager.updateSaveSlots();
                        saveSlotInfo = saveManager.getSaveSlotInfo();
                        selectedSaveSlot = 0;
                        loadSlotMenuInitialized = true;
                        loadSlotFromMainMenu = true;
                        currentState = GameState::LOAD_SLOT_SELECTION;
                        spdlog::info("Entering load slot selection from main menu");
                    } else if (selectedMenuOption == 2) {
                        // Exit game
                        glfwSetWindowShouldClose(window, GLFW_TRUE);
                    }
                } else {
                    // Menu without save file: Start Game, Exit Game
                    if (selectedMenuOption == 0) {
                        // Start new game - reset game state
                        spdlog::info("Starting new game");
                        gameInitialized = false; // Force re-initialization
                        currentState = GameState::PLAYING;
                    } else if (selectedMenuOption == 1) {
                        // Exit game
                        glfwSetWindowShouldClose(window, GLFW_TRUE);
                    }
                }
                
                // Start background music when entering game
                if (currentState == GameState::PLAYING) {
                    // Stop intro music when starting game
                    audioManager->stopMusic();
                    introMusicStarted = false;
                    // Set lower volume for background music during gameplay
                    audioManager->setMusicVolume(0.4f); // Reduced from default 1.0 to 0.4
                    // Start background music for gameplay
                    audioManager->playMusic("background", true); // Loop the background music
                    backgroundMusicStarted = true;
                    spdlog::info("Started background music for gameplay at reduced volume (0.4)");
                    
                    // Load game if continuing from save
                    if (hasSaveFile && (selectedMenuOption == 1 || selectedMenuOption == 2)) {
                        SaveData saveData;
                        int mostRecentSlot = saveManager.getMostRecentSaveSlot();
                        if (mostRecentSlot >= 0 && saveManager.loadGame(saveData, mostRecentSlot)) {
                            // Load the game state using GameplayManager
                            gameplayManager.loadGame(saveData, initializer.getAssetPath(""));
                                gameInitialized = true;
                            spdlog::info("Game loaded from main menu");
                        } else {
                            spdlog::error("Failed to load game from main menu");
                        }
                    }
                }
                
                keyEnterPressed = true;
            } else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                keyEnterPressed = false;
            }

            // Draw menu
            UI::drawMainMenu(windowWidth, windowHeight, selectedMenuOption, hasSaveFile);
        }
        else if (currentState == GameState::PLAYING) {
            // Initialize game if not already done
            if (!gameplayManager.isGameInitialized()) {
                gameplayManager.startNewGame();
                gameInitialized = true;
                spdlog::info("Game initialized successfully");
                
                // Load sound effects (only load what's available)
                if (!audioManager->loadSound("intro", initializer.getAssetPath("assets/sounds/intro.wav"))) {
                    spdlog::warn("Failed to load intro sound");
                }
            }

            // Update and draw gameplay
            gameplayManager.update(deltaTime, window, windowWidth, windowHeight);
            gameplayManager.draw(windowWidth, windowHeight);

            // Check for ESC key to pause game
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !keyEscPressed) {
                selectedPauseButton = 0;
                pauseScreenInitialized = false;
                keyUpPressed = false;
                keyDownPressed = false;
                keyEnterPressed = false;
                // Reset hover tracking for pause screen
                previousSelectedPauseButton = -1;
                currentState = GameState::PAUSED;
                keyEscPressed = true;
                spdlog::info("Game paused");
            } else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
                keyEscPressed = false;
            }
            
            // Check if player has died
            if (!gameplayManager.isPlayerAlive()) {
                // audioManager->playSound("player_death", 1.0f);
                // Stop background music when player dies
                audioManager->stopMusic();
                backgroundMusicStarted = false;
                // Reset music volume to normal for intro music
                audioManager->setMusicVolume(1.0f);
                spdlog::info("Stopped background music due to player death");
                selectedDeathButton = 0;
                deathScreenInitialized = false;
                keyUpPressed = false;
                keyDownPressed = false;
                keyEnterPressed = false;
                // Reset hover tracking for death screen
                previousSelectedDeathButton = -1;
                previousRespawnButtonHovered = false;
                previousExitButtonHovered = false;
                currentState = GameState::DEATH;
                spdlog::info("Player has died, showing death screen");
            }
        }
        else if (currentState == GameState::PAUSED) {
            // Handle pause menu input
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
            if (selectedPauseButton != previousSelectedPauseButton) {
                if (!hoverSoundPlayed) {
                    uiAudioManager->playButtonHoverSound();
                    hoverSoundPlayed = true;
                }
                previousSelectedPauseButton = selectedPauseButton;
            } else {
                hoverSoundPlayed = false; // Reset when not changing
            }
            
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed) {
                uiAudioManager->playButtonClickSound();
                
                if (selectedPauseButton == 0) {
                    // Resume game
                    currentState = GameState::PLAYING;
                    spdlog::info("Resuming game");
                } else if (selectedPauseButton == 1) {
                    // Save game - go to save slot selection
                    saveManager.updateSaveSlots();
                    saveSlotInfo = saveManager.getSaveSlotInfo();
                    selectedSaveSlot = 0;
                    saveSlotMenuInitialized = true;
                    currentState = GameState::SAVE_SLOT_SELECTION;
                    spdlog::info("Entering save slot selection");
                } else if (selectedPauseButton == 2) {
                    // Back to main menu
                    // Stop background music and reset introMusicStarted flag
                    audioManager->stopMusic();
                    introMusicStarted = false;
                    backgroundMusicStarted = false;
                    // Reset music volume to normal for intro music
                    audioManager->setMusicVolume(1.0f);
                    // Reset menu hover tracking
                    previousSelectedMenuOption = -1;
                    currentState = GameState::MENU;
                    spdlog::info("Returning to main menu from pause");
                } else if (selectedPauseButton == 3) {
                    // Exit game
                    spdlog::info("Exiting game from pause menu");
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

            // Draw the game in the background (paused state) - NO UPDATES, just drawing
            if (gameplayManager.isGameInitialized()) {
                gameplayManager.drawPaused(windowWidth, windowHeight);
            }
            
            // Draw pause menu overlay
            UI::drawPauseScreen(windowWidth, windowHeight, selectedPauseButton);
        }
        else if (currentState == GameState::SAVE_SLOT_SELECTION) {
            // Draw game in background (paused)
            if (gameplayManager.isGameInitialized()) {
                gameplayManager.drawPaused(windowWidth, windowHeight);
            }
            
            // Draw save slot selection menu
            UI::drawSaveSlotMenu(windowWidth, windowHeight, selectedSaveSlot, saveSlotInfo);
        }
        else if (currentState == GameState::LOAD_SLOT_SELECTION) {
            // Draw game in background (paused)
            if (gameplayManager.isGameInitialized()) {
                gameplayManager.drawPaused(windowWidth, windowHeight);
            }
            
            // Draw load slot selection menu
            UI::drawLoadSlotMenu(windowWidth, windowHeight, selectedSaveSlot, saveSlotInfo);
        }
        
        // Input handling for save slot selection states
        if (currentState == GameState::SAVE_SLOT_SELECTION) {
            // Handle save slot selection input
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
                    if (gameplayManager.isGameInitialized()) {
                        SaveData saveData = gameplayManager.createSaveData();
                        
                        if (saveManager.saveGame(saveData, selectedSaveSlot)) {
                            saveManager.updateSaveSlots();
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
        else if (currentState == GameState::LOAD_SLOT_SELECTION) {
            // Handle load slot selection input
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
                if (selectedSaveSlot < 3 && saveManager.getSaveSlot(selectedSaveSlot).hasSave()) {
                    // Load from selected slot
                    SaveData saveData;
                    if (saveManager.loadGame(saveData, selectedSaveSlot)) {
                        // Load the game state using GameplayManager
                        gameplayManager.loadGame(saveData, initializer.getAssetPath(""));
                        
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
        else if (currentState == GameState::DEATH) {
            // Handle mouse input for death screen
            glfwGetCursorPos(window, &mouseX, &mouseY);
            float buttonWidth = 260.0f;  // Increased from 200 to 250 to match UI
            float buttonHeight = 60.0f;
            float buttonX = windowWidth / 2.0f - buttonWidth / 2.0f - 45.0f;  // Match UI position (45px left offset)
            float respawnButtonY = windowHeight * 0.5f;
            float exitButtonY = windowHeight * 0.35f;

            bool respawnButtonHovered = UI::isMouseOverButton(mouseX, mouseY, buttonX, respawnButtonY, buttonWidth, buttonHeight);
            bool exitButtonHovered = UI::isMouseOverButton(mouseX, mouseY, buttonX, exitButtonY, buttonWidth, buttonHeight);

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
            if (selectedDeathButton != previousSelectedDeathButton) {
                if (!hoverSoundPlayed) {
                uiAudioManager->playButtonHoverSound();
                    hoverSoundPlayed = true;
                }
                previousSelectedDeathButton = selectedDeathButton;
            } else {
                hoverSoundPlayed = false; // Reset when not changing
            }

            // Mouse hover does NOT affect selection anymore
            // if (respawnButtonHovered) selectedDeathButton = 0;
            // if (exitButtonHovered) selectedDeathButton = 1;

            // Mouse click (still works for clicking)
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mouseLeftPressed) {
                if (respawnButtonHovered) {
                    // Play click sound before respawning
                    uiAudioManager->playButtonClickSound();
                    // Reset game state
                    spdlog::info("Respawn button clicked, restarting game");
                    gameplayManager.resetGame();
                        gameInitialized = false;
                    deathScreenInitialized = false; // <-- FIX: reset on respawn
                    // Start background music for gameplay
                    audioManager->setMusicVolume(0.4f); // Set lower volume for background music
                    audioManager->playMusic("background", true); // Loop the background music
                    backgroundMusicStarted = true;
                    spdlog::info("Started background music for gameplay at reduced volume (0.4)");
                    currentState = GameState::PLAYING;
                } else if (exitButtonHovered) {
                    // Play click sound before exiting
                    uiAudioManager->playButtonClickSound();
                    spdlog::info("Exit button clicked, exiting game");
                    deathScreenInitialized = false; // <-- FIX: reset on exit
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
                    uiAudioManager->playButtonClickSound();
                    spdlog::info("Enter pressed on Respawn, restarting game");
                    gameplayManager.resetGame();
                        gameInitialized = false;
                    deathScreenInitialized = false; // <-- FIX: reset on respawn
                    // Start background music for gameplay
                    audioManager->setMusicVolume(0.4f); // Set lower volume for background music
                    audioManager->playMusic("background", true); // Loop the background music
                    backgroundMusicStarted = true;
                    spdlog::info("Started background music for gameplay at reduced volume (0.4)");
                    currentState = GameState::PLAYING;
                } else if (selectedDeathButton == 1) {
                    // Play click sound before exiting
                    uiAudioManager->playButtonClickSound();
                    spdlog::info("Enter pressed on Exit, exiting game");
                    deathScreenInitialized = false; // <-- FIX: reset on exit
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
                keyEnterPressed = true;
            } else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                keyEnterPressed = false;
            }

            // Draw death screen with both buttons
            UI::drawDeathScreen(windowWidth, windowHeight, respawnButtonHovered, exitButtonHovered, selectedDeathButton);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup is handled by GameplayManager destructor

    // Cleanup UI system
    UI::cleanup();
    
    // Cleanup projectile texture
    Projectile::cleanupProjectileTexture();

    spdlog::info("Shutting down Ortos II application");
    // GameInitializer will handle cleanup in its destructor
    return 0;
}