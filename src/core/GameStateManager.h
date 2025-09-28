#pragma once
#include <GLFW/glfw3.h>
#include "core/GameplayManager.h"
#include "save/EnhancedSaveManager.h"
#include "audio/AudioManager.h"
#include "audio/UIAudioManager.h"
#include "config/ConfigManager.h"
#include "ui/UI.h"

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    DEATH,
    SAVE_SLOT_SELECTION,
    LOAD_SLOT_SELECTION,
    SETTINGS
};

class CoreGameStateManager {
public:
    CoreGameStateManager();
    ~CoreGameStateManager();

    // Initialization
    void initialize(GameplayManager* gameplayManager, 
                    EnhancedSaveManager* saveManager,
                    AudioManager* audioManager,
                    UIAudioManager* uiAudioManager,
                    ConfigManager* configManager,
                    GLFWwindow* window,
                    const std::string& assetPath);

    // Main update and draw methods
    void update(float deltaTime, int windowWidth, int windowHeight);
    void draw(int windowWidth, int windowHeight);

    // State management
    GameState getCurrentState() const { return currentState; }
    void setCurrentState(GameState state) { currentState = state; }
    bool isGameInitialized() const { return gameInitialized; }

    // Cleanup
    void cleanup();

private:
    // Core components
    GameplayManager* gameplayManager;
    EnhancedSaveManager* saveManager;
    AudioManager* audioManager;
    UIAudioManager* uiAudioManager;
    ConfigManager* configManager;
    GLFWwindow* window;
    std::string assetPath;

    // Game state
    GameState currentState;
    bool gameInitialized;
    bool hasSaveFile;

    // Menu state
    int selectedMenuOption;
    int previousSelectedMenuOption;

    // Pause menu state
    int selectedPauseButton;
    int previousSelectedPauseButton;

    // Save/Load slot state
    int selectedSaveSlot;
    std::vector<std::string> saveSlotInfo;
    bool loadSlotFromMainMenu;

    // Death screen state
    int selectedDeathButton;
    int previousSelectedDeathButton;
    double mouseX, mouseY;
    bool respawnButtonHovered;
    bool exitButtonHovered;
    bool previousRespawnButtonHovered;
    bool previousExitButtonHovered;

    // Settings state
    int selectedSettingsOption;
    int previousSelectedSettingsOption;
    float masterVolume;
    float musicVolume;
    float sfxVolume;
    GameState previousState; // To remember where we came from
    
    // Input timing for settings
    float volumeAdjustTimer;
    float volumeAdjustDelay;

    // Audio state
    bool introMusicStarted;
    bool backgroundMusicStarted;

    // Input state
    bool keyUpPressed;
    bool keyDownPressed;
    bool keyEnterPressed;
    bool keyEscPressed;
    bool mouseLeftPressed;
    bool hoverSoundPlayed;
    bool clickSoundPlayed;

    // State handlers
    void handleMenuState(int windowWidth, int windowHeight);
    void handlePlayingState(float deltaTime, int windowWidth, int windowHeight);
    void handlePausedState(int windowWidth, int windowHeight);
    void handleDeathState(int windowWidth, int windowHeight);
    void handleSaveSlotSelectionState(int windowWidth, int windowHeight);
    void handleLoadSlotSelectionState(int windowWidth, int windowHeight);
    void handleSettingsState(int windowWidth, int windowHeight);

    // Input handling
    void updateInputStates();
    void resetInputStates();
    void handleMenuInput();
    void handlePauseInput();
    void handleDeathInput();
    void handleSaveSlotInput();
    void handleLoadSlotInput();
    void handleSettingsInput();

    // Audio management
    void startIntroMusic();
    void startGameplayMusic();
    void stopMusic();
    void resetMusicState();

    // Settings management
    void loadSettings();
    void saveSettings();

    // State transitions
    void transitionToMenu();
    void transitionToPlaying();
    void transitionToPaused();
    void transitionToDeath();
    void transitionToSaveSlotSelection();
    void transitionToLoadSlotSelection();
    void transitionToSettings();

    // Helper methods
    void updateSaveSlots();
    void playHoverSound(int currentSelection, int& previousSelection);
    void playClickSound();
    bool isMouseOverButton(double mouseX, double mouseY, float buttonX, float buttonY, float buttonWidth, float buttonHeight);
};
