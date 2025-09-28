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
#include "save/EnhancedSaveManager.h"
#include "save/GameStateManager.h"
#include "collision/CollisionManager.h"
#include "core/GameInitializer.h"
#include "core/GameplayManager.h"
#include "core/GameStateManager.h"
#include "config/ConfigManager.h"
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
// GameState enum is now in GameStateManager.h


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
    
    // Initialize enhanced save manager with database support
    EnhancedSaveManager saveManager(initializer.getAssetPath("saves/"));
    saveManager.initialize();
    
    // Initialize config manager for settings
    ConfigManager configManager;
    configManager.initialize(initializer.getAssetPath("config/game_config.cfg"));
    
    // Initialize gameplay manager
    GameplayManager gameplayManager;
    if (!gameplayManager.initialize(initializer.getAssetPath(""), audioManager, uiAudioManager)) {
        spdlog::error("Failed to initialize gameplay manager");
        return -1;
    }
    
    // Set save manager for gameplay manager
    gameplayManager.setSaveManager(&saveManager);

    // Initialize game state manager
    CoreGameStateManager gameStateManager;
    gameStateManager.initialize(&gameplayManager, &saveManager, audioManager, uiAudioManager, &configManager, window, initializer.getAssetPath(""));

    // Get window dimensions
    int windowWidth = 1920;
    int windowHeight = 1080;

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT);

        // Update game state manager
        gameStateManager.update(deltaTime, windowWidth, windowHeight);
        
        // Draw current state
        gameStateManager.draw(windowWidth, windowHeight);

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
