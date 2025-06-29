#include <GLFW/glfw3.h>
#include <AL/al.h>
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
#include "BloodEffect.h"
#include "AudioManager.h"
#include "UIAudioManager.h"
#include "InputHandler.h"
#include "TileMap.h"
#include "UI.h"
#include <iostream>
#include "nlohmann/json.hpp"
#include <stb_image.h>
#include <spdlog/spdlog.h>
#include <algorithm>
using json = nlohmann::json;

enum class GameState {
    MENU,
    PLAYING,
    DEATH
};

int main() {
    // Initialize logger
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    spdlog::info("Starting Ortos II application");
    if (!glfwInit()) {
        spdlog::error("Failed to initialize GLFW");
        return -1;
    }

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Ortos II", primaryMonitor, nullptr);

    if (!window) {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
    glfwMakeContextCurrent(window);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); 
    glEnable(GL_TEXTURE_2D); 

    // Initialize UI system with FreeType
    if (!UI::init("../assets/fonts/pixel.ttf")) {
        spdlog::error("Failed to initialize UI system");
        glfwTerminate();
        return -1;
    }

    // Load title screen background texture
    if (!UI::loadTitleScreenTexture("../assets/screens/titlescreen.png")) {
        spdlog::warn("Failed to load title screen texture, will use black background");
    }

    // Load death screen background texture
    if (!UI::loadDeathScreenTexture("../assets/screens/deathscreen.png")) {
        spdlog::warn("Failed to load death screen texture, will use black background");
    }

    // Initialize AudioManager
    AudioManager audioManager;
    spdlog::info("Attempting to initialize AudioManager...");
    if (!audioManager.init()) {
        spdlog::error("Failed to initialize AudioManager");
        glfwTerminate();
        return -1;
    }
    spdlog::info("AudioManager initialized successfully");

    // Initialize UIAudioManager
    UIAudioManager uiAudioManager;
    spdlog::info("Attempting to initialize UIAudioManager...");
    if (!uiAudioManager.init(audioManager.getContext())) {
        spdlog::error("Failed to initialize UIAudioManager");
        glfwTerminate();
        return -1;
    }
    spdlog::info("UIAudioManager initialized successfully");

    // Load UI sound effects
    spdlog::info("Attempting to load UI sound effects...");
    if (!uiAudioManager.loadUISound("button", "../assets/sounds/button.wav")) {
        spdlog::warn("Failed to load button sound");
    } else {
        spdlog::info("Successfully loaded button sound");
    }

    // Load intro music for title screen
    spdlog::info("Attempting to load intro music...");
    if (!audioManager.loadMusic("intro", "../assets/sounds/intro.wav")) {
        spdlog::warn("Failed to load intro music");
    } else {
        spdlog::info("Successfully loaded intro music");
    }

    // Load background music for gameplay
    spdlog::info("Attempting to load background music...");
    if (!audioManager.loadMusic("background", "../assets/sounds/defaultSong.wav")) {
        spdlog::warn("Failed to load background music");
    } else {
        spdlog::info("Successfully loaded background music");
    }

    // Set up viewport and orthographic projection
    int windowWidth = 1920;
    int windowHeight = 1080;
    glfwSetWindowSize(window, windowWidth, windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    // Game state management
    GameState currentState = GameState::MENU;
    int selectedMenuOption = 0;
    bool gameInitialized = false;
    bool introMusicStarted = false;
    bool backgroundMusicStarted = false;
    
    // Input debouncing
    bool keyUpPressed = false;
    bool keyDownPressed = false;
    bool keyEnterPressed = false;
    
    // Mouse input for death screen
    double mouseX = 0.0, mouseY = 0.0;
    bool mouseLeftPressed = false;
    bool respawnButtonHovered = false;
    
    // Button hover tracking for sound effects
    int previousSelectedMenuOption = -1;
    int previousSelectedDeathButton = -1;
    bool previousRespawnButtonHovered = false;
    bool previousExitButtonHovered = false;

    // Game objects (will be initialized when starting game)
    Player* player = nullptr;
    Enemy* enemy = nullptr;
    std::vector<Projectile> playerProjectiles;
    std::vector<Projectile> enemyProjectiles;
    std::vector<BloodEffect*> bloodEffects;
    InputHandler* inputHandler = nullptr;
    Tilemap* tilemap = nullptr;

    float lastTime = glfwGetTime();

    // --- Add these at the top of main, after other variables ---
    int selectedDeathButton = 0;
    bool deathScreenInitialized = false;

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT);

        if (currentState == GameState::MENU) {
            // Start intro music if not already started
            if (!introMusicStarted) {
                audioManager.playMusic("intro", true); // Loop the intro music
                introMusicStarted = true;
                spdlog::info("Started intro music");
            }
            
            // Handle menu input
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !keyUpPressed) {
                selectedMenuOption = (selectedMenuOption - 1 + 2) % 2;
                keyUpPressed = true;
            } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
                keyUpPressed = false;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !keyDownPressed) {
                selectedMenuOption = (selectedMenuOption + 1) % 2;
                keyDownPressed = true;
            } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
                keyDownPressed = false;
            }
            
            // Play hover sound when selection changes
            if (selectedMenuOption != previousSelectedMenuOption) {
                uiAudioManager.playButtonHoverSound();
                previousSelectedMenuOption = selectedMenuOption;
            }
            
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed) {
                if (selectedMenuOption == 0) {
                    // Play click sound before starting game
                    uiAudioManager.playButtonClickSound();
                    // Stop intro music when starting game
                    audioManager.stopMusic();
                    introMusicStarted = false;
                    // Start background music for gameplay
                    audioManager.playMusic("background", true); // Loop the background music
                    backgroundMusicStarted = true;
                    spdlog::info("Started background music for gameplay");
                    currentState = GameState::PLAYING;
                } else if (selectedMenuOption == 1) {
                    // Play click sound before exiting
                    uiAudioManager.playButtonClickSound();
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
                keyEnterPressed = true;
            } else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
                keyEnterPressed = false;
            }

            // Draw menu
            UI::drawMainMenu(windowWidth, windowHeight, selectedMenuOption);
        }
        else if (currentState == GameState::PLAYING) {
            // Initialize game objects if not already done
            if (!gameInitialized) {
                player = new Player();
                stbi_set_flip_vertically_on_load(true);
                player->loadTexture("../assets/graphic/Vampire_Walk.png", 64, 64, 4);
                player->loadIdleTexture("../assets/graphic/Vampire_Idle.png", 64, 64, 2);
                stbi_set_flip_vertically_on_load(false);
                
                // Create enemy
                enemy = new Enemy(20 * 16.0f, 15 * 16.0f, EnemyType::Skeleton);
                stbi_set_flip_vertically_on_load(true);
                enemy->loadTexture("../assets/graphic/skeleton_enemy.png", 64, 64, 4);
                stbi_set_flip_vertically_on_load(false);
                
                inputHandler = new InputHandler();
                tilemap = new Tilemap();
                if (!tilemap->loadTilesetTexture("../assets/maps/catacombs.png", 16, 16)) {
                    spdlog::error("Failed to load tileset texture");
                    return -1;
                }
                if (!tilemap->loadFromJSON("../assets/maps/test.json")) {
                    spdlog::error("Failed to load map from JSON.");
                    return -1;
                }

                // Set up projection to match tilemap size
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                float mapWidth = tilemap->getWidthInTiles() * tilemap->getTileWidth();
                float mapHeight = tilemap->getHeightInTiles() * tilemap->getTileHeight();
                glOrtho(0.0, mapWidth, mapHeight, 0.0, -1.0, 1.0);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

                gameInitialized = true;
                spdlog::info("Game initialized successfully");
                
                // Load sound effects (only load what's available)
                if (!audioManager.loadSound("intro", "../assets/sounds/intro.wav")) {
                    spdlog::warn("Failed to load intro sound");
                }
            }

            // Game logic
            inputHandler->processInput(window, *player, deltaTime, *tilemap, playerProjectiles);
            
            // Play shoot sound if player shot
            if (playerProjectiles.size() > 0 && playerProjectiles.back().isActive()) {
                // audioManager.playSound("shoot", 0.7f); // Commented out - no shoot sound available
            }
        
            tilemap->draw();
            
            // Update and draw blood effects (ground layer)
            for (auto& bloodEffect : bloodEffects) {
                bloodEffect->update(deltaTime);
                bloodEffect->draw();
            }
            
            player->draw();
            
            // Update and draw enemy
            enemy->update(deltaTime, player->getX(), player->getY(), *tilemap);
            enemy->updateAnimation(deltaTime);
            enemy->draw();
            
            // Enemy shooting
            enemy->shootProjectile(player->getX(), player->getY(), enemyProjectiles);
            
            // Update and draw projectiles
            for (auto& projectile : playerProjectiles) {
                projectile.update(deltaTime);
                
                // Check for wall collision
                if (projectile.checkWallCollision(*tilemap)) {
                    projectile.setActive(false);
                    spdlog::info("Player projectile destroyed by wall collision");
                }
                
                projectile.draw();
            }
            
            for (auto& projectile : enemyProjectiles) {
                projectile.update(deltaTime);
                
                // Check for wall collision
                if (projectile.checkWallCollision(*tilemap)) {
                    projectile.setActive(false);
                    spdlog::info("Enemy projectile destroyed by wall collision");
                }
                
                projectile.draw();
            }
            
            // Collision detection
            // Player projectiles vs Enemy
            for (auto& projectile : playerProjectiles) {
                if (projectile.isActive() && enemy->isAlive()) {
                    if (projectile.checkCollision(enemy->getX(), enemy->getY(), 8.0f)) {
                        projectile.setActive(false);
                        enemy->takeDamage(25);  // Deal 25 damage
                        // audioManager.playSound("enemy_hit", 0.8f);
                        spdlog::info("Enemy hit by player projectile! Enemy HP: {}/{}", 
                                    enemy->getCurrentHealth(), enemy->getMaxHealth());
                    }
                }
            }
            
            // Enemy projectiles vs Player
            for (auto& projectile : enemyProjectiles) {
                if (projectile.isActive()) {
                    if (projectile.checkCollision(player->getX(), player->getY(), 8.0f)) {
                        projectile.setActive(false);
                        player->takeDamage(15);  // Deal 15 damage
                        // audioManager.playSound("player_hit", 0.6f);
                        spdlog::info("Player hit by enemy projectile! Player HP: {}/{}", 
                                    player->getCurrentHealth(), player->getMaxHealth());
                    }
                }
            }
            
            // Blood effect creation when enemy dies
            if (enemy->shouldCreateBloodEffect()) {
                bloodEffects.push_back(new BloodEffect(enemy->getX(), enemy->getY()));
                enemy->markBloodEffectCreated();
                // audioManager.playSound("enemy_death", 1.0f);
                spdlog::info("Blood effect created at enemy death position ({}, {})", enemy->getX(), enemy->getY());
            }
            
            // Clean up inactive projectiles
            playerProjectiles.erase(
                std::remove_if(playerProjectiles.begin(), playerProjectiles.end(),
                    [](const Projectile& p) { return !p.isActive(); }),
                playerProjectiles.end()
            );
            
            enemyProjectiles.erase(
                std::remove_if(enemyProjectiles.begin(), enemyProjectiles.end(),
                    [](const Projectile& p) { return !p.isActive(); }),
                enemyProjectiles.end()
            );
            
            // Draw UI (player health bar) LAST so it's always on top
            UI::drawPlayerHealth(player->getCurrentHealth(), player->getMaxHealth(), windowWidth, windowHeight);

            // Check for ESC key to return to menu
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                // Stop background music and reset introMusicStarted flag
                audioManager.stopMusic();
                introMusicStarted = false;
                backgroundMusicStarted = false;
                // Reset menu hover tracking
                previousSelectedMenuOption = -1;
                currentState = GameState::MENU;
                spdlog::info("Returning to menu");
            }
            
            // Check if player has died
            if (!player->isAlive()) {
                // audioManager.playSound("player_death", 1.0f);
                // Stop background music when player dies
                audioManager.stopMusic();
                backgroundMusicStarted = false;
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

            // Play hover sound when selection changes
            if (selectedDeathButton != previousSelectedDeathButton) {
                uiAudioManager.playButtonHoverSound();
                previousSelectedDeathButton = selectedDeathButton;
            }

            // Mouse hover does NOT affect selection anymore
            // if (respawnButtonHovered) selectedDeathButton = 0;
            // if (exitButtonHovered) selectedDeathButton = 1;

            // Mouse click (still works for clicking)
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mouseLeftPressed) {
                if (respawnButtonHovered) {
                    // Play click sound before respawning
                    uiAudioManager.playButtonClickSound();
                    // Reset game state
                    spdlog::info("Respawn button clicked, restarting game");
                    if (gameInitialized) {
                        delete player;
                        delete enemy;
                        delete inputHandler;
                        delete tilemap;
                        gameInitialized = false;
                    }
                    playerProjectiles.clear();
                    enemyProjectiles.clear();
                    // Clean up blood effects
                    for (auto& bloodEffect : bloodEffects) {
                        delete bloodEffect;
                    }
                    bloodEffects.clear();
                    deathScreenInitialized = false; // <-- FIX: reset on respawn
                    // Start background music for gameplay
                    audioManager.playMusic("background", true); // Loop the background music
                    backgroundMusicStarted = true;
                    spdlog::info("Started background music for respawned gameplay");
                    currentState = GameState::PLAYING;
                } else if (exitButtonHovered) {
                    // Play click sound before exiting
                    uiAudioManager.playButtonClickSound();
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
                    uiAudioManager.playButtonClickSound();
                    spdlog::info("Enter pressed on Respawn, restarting game");
                    if (gameInitialized) {
                        delete player;
                        delete enemy;
                        delete inputHandler;
                        delete tilemap;
                        gameInitialized = false;
                    }
                    playerProjectiles.clear();
                    enemyProjectiles.clear();
                    // Clean up blood effects
                    for (auto& bloodEffect : bloodEffects) {
                        delete bloodEffect;
                    }
                    bloodEffects.clear();
                    deathScreenInitialized = false; // <-- FIX: reset on respawn
                    // Start background music for gameplay
                    audioManager.playMusic("background", true); // Loop the background music
                    backgroundMusicStarted = true;
                    spdlog::info("Started background music for respawned gameplay (keyboard)");
                    currentState = GameState::PLAYING;
                } else if (selectedDeathButton == 1) {
                    // Play click sound before exiting
                    uiAudioManager.playButtonClickSound();
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

    // Cleanup
    if (gameInitialized) {
        delete player;
        delete enemy;
        delete inputHandler;
        delete tilemap;
    }
    
    // Clean up blood effects
    for (auto& bloodEffect : bloodEffects) {
        delete bloodEffect;
    }
    bloodEffects.clear();

    // Cleanup UI system
    UI::cleanup();

    spdlog::info("Shutting down Ortos II application");
    glfwTerminate();
    return 0;
}