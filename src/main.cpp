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
    // Level management
    std::string currentLevelPath = initializer.getAssetPath("assets/maps/test.json");
    std::string nextLevelPath = initializer.getAssetPath("assets/maps/final.json");
    float levelTransitionCooldown = 0.0f;
    
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

    // Game objects (will be initialized when starting game)
    Player* player = nullptr;
    std::vector<Enemy*> enemies;
    std::vector<Projectile> playerProjectiles;
    std::vector<Projectile> enemyProjectiles;
    std::vector<BloodEffect*> bloodEffects;
    InputHandler* inputHandler = nullptr;
    Tilemap* tilemap = nullptr;

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
                            // Initialize game objects first if not already done
                            if (!gameInitialized) {
                                // Initialize all game objects properly
                                player = new Player();
                                stbi_set_flip_vertically_on_load(true);
                                player->loadTexture(initializer.getAssetPath("assets/graphic/enemies/vampire/Vampire_Walk.png"), 64, 64, 4);
                                player->loadIdleTexture(initializer.getAssetPath("assets/graphic/enemies/vampire/Vampire_Idle.png"), 64, 64, 2);
                                stbi_set_flip_vertically_on_load(false);
                                
                                inputHandler = new InputHandler();
                                tilemap = new Tilemap();
                                if (!tilemap->loadTilesetTexture(initializer.getAssetPath("assets/graphic/tileset/tileset.png"), 16, 16)) {
                                    spdlog::error("Failed to load tileset texture");
                                    return -1;
                                }
                                
                                // Load projectile texture
                                Projectile::loadProjectileTexture(initializer.getAssetPath("assets/graphic/projectiles/green_projectiles.png"));
                                
                                gameInitialized = true;
                            }
                            // Load the game state
                            spdlog::info("=== MAIN: About to call GameStateManager::loadGameState (from main menu) ===");
                            GameStateManager::loadGameState(saveData, player, enemies, playerProjectiles, enemyProjectiles, currentLevelPath, levelTransitionCooldown, initializer.getAssetPath(""));
                            spdlog::info("=== MAIN: GameStateManager::loadGameState completed (from main menu) ===");
                            
                            // Reload the tilemap for the saved level
                            if (tilemap) {
                                delete tilemap;
                            }
                            tilemap = new Tilemap();
                            if (!tilemap->loadTilesetTexture(initializer.getAssetPath("assets/graphic/tileset/tileset.png"), 16, 16)) {
                                spdlog::error("Failed to load tileset texture");
                                return -1;
                            }
                            if (!tilemap->loadFromJSON(currentLevelPath)) {
                                spdlog::error("Failed to load tilemap for saved level: {}", currentLevelPath);
                                // Fallback to default level
                                tilemap->loadFromJSON(initializer.getAssetPath("assets/levels/level1.json"));
                                currentLevelPath = initializer.getAssetPath("assets/levels/level1.json");
                            }
                            
                            // Set up projection to match tilemap size
                            glMatrixMode(GL_PROJECTION);
                            glLoadIdentity();
                            float mapWidth = tilemap->getWidthInTiles() * tilemap->getTileWidth();
                            float mapHeight = tilemap->getHeightInTiles() * tilemap->getTileHeight();
                            glOrtho(0.0, mapWidth, mapHeight, 0.0, -1.0, 1.0);
                            glMatrixMode(GL_MODELVIEW);
                            glLoadIdentity();
                            
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
            // Initialize game objects if not already done
            if (!gameInitialized) {
                spdlog::info("Initializing new game objects...");
                // Clean up any existing game objects first
                if (player) {
                    delete player;
                    player = nullptr;
                }
                for (auto& enemy : enemies) {
                    if (enemy) {
                        delete enemy;
                    }
                }
                enemies.clear();
                if (inputHandler) {
                    delete inputHandler;
                    inputHandler = nullptr;
                }
                if (tilemap) {
                    delete tilemap;
                    tilemap = nullptr;
                }
                playerProjectiles.clear();
                enemyProjectiles.clear();
                
                spdlog::info("Creating player...");
                player = new Player();
                stbi_set_flip_vertically_on_load(true);
                spdlog::info("Loading player textures...");
                player->loadTexture(initializer.getAssetPath("assets/graphic/enemies/vampire/Vampire_Walk.png"), 64, 64, 4);
                player->loadIdleTexture(initializer.getAssetPath("assets/graphic/enemies/vampire/Vampire_Idle.png"), 64, 64, 2);
                stbi_set_flip_vertically_on_load(false);
                
                // Create enemies
                spdlog::info("Creating enemies...");
                // Flying eye enemy
                Enemy* flyingEye = new Enemy(25 * 16.0f, 10 * 16.0f, EnemyType::FlyingEye);
                stbi_set_flip_vertically_on_load(true);
                spdlog::info("Loading flying eye textures...");
                flyingEye->loadTexture(initializer.getAssetPath("assets/graphic/enemies/flying_eye/flgyingeye.png"), 150, 150, 8);
                flyingEye->loadHitTexture(initializer.getAssetPath("assets/graphic/enemies/flying_eye/Hit_eye.png"), 150, 150, 4);
                flyingEye->loadDeathTexture(initializer.getAssetPath("assets/graphic/enemies/flying_eye/Death_eye.png"), 150, 150, 4); // NEW
                stbi_set_flip_vertically_on_load(false);
                enemies.push_back(flyingEye);
                
                // Shroom enemy
                Enemy* shroom = new Enemy(15 * 16.0f, 12 * 16.0f, EnemyType::Shroom);
                stbi_set_flip_vertically_on_load(true);
                spdlog::info("Loading shroom textures...");
                shroom->loadTexture(initializer.getAssetPath("assets/graphic/enemies/shroom/shroom.png"), 150, 150, 8);
                shroom->loadHitTexture(initializer.getAssetPath("assets/graphic/enemies/shroom/Hit_shroom.png"), 150, 150, 4);
                shroom->loadDeathTexture(initializer.getAssetPath("assets/graphic/enemies/shroom/Death_shroom.png"), 150, 150, 4); // NEW
                stbi_set_flip_vertically_on_load(false);
                enemies.push_back(shroom);
                
                spdlog::info("Creating input handler and tilemap...");
                inputHandler = new InputHandler();
                tilemap = new Tilemap();
                spdlog::info("Loading tileset texture...");
                if (!tilemap->loadTilesetTexture(initializer.getAssetPath("assets/graphic/tileset/tileset.png"), 16, 16)) {
                    spdlog::error("Failed to load tileset texture");
                    return -1;
                }
                spdlog::info("Loading map from JSON: {}", currentLevelPath);
                if (!tilemap->loadFromJSON(currentLevelPath)) {
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

                // Load projectile texture
                spdlog::info("Loading projectile textures...");
                Projectile::loadProjectileTexture(initializer.getAssetPath("assets/graphic/projectiles/green_projectiles.png"));
                
                gameInitialized = true;
                spdlog::info("Game initialized successfully");
                
                // Load sound effects (only load what's available)
                if (!audioManager->loadSound("intro", initializer.getAssetPath("assets/sounds/intro.wav"))) {
                    spdlog::warn("Failed to load intro sound");
                }
            }

            // Game logic
            // Decrease level transition cooldown
            if (levelTransitionCooldown > 0.0f) {
                levelTransitionCooldown -= deltaTime;
            }

            // Gate open if no enemies are alive
            bool anyEnemyAliveForMove = std::any_of(enemies.begin(), enemies.end(), [](Enemy* e){ return e && e->isAlive(); });
            bool gateOpenForMove = !anyEnemyAliveForMove;
            inputHandler->processInput(window, *player, deltaTime, *tilemap, playerProjectiles, gateOpenForMove);

            // Gate trigger: step on tiles with IDs 120,121,122,123
            {
                int tileW = tilemap->getTileWidth();
                int tileH = tilemap->getTileHeight();
                int playerTileX = static_cast<int>(player->getX() / tileW);
                int playerTileY = static_cast<int>(player->getY() / tileH);
                int gid = tilemap->getNormalizedTileIdAt(playerTileX, playerTileY);
                bool onGate = (gid == 120 || gid == 121 || gid == 122 || gid == 123);
                bool anyEnemyAlive = std::any_of(enemies.begin(), enemies.end(), [](Enemy* e){ return e && e->isAlive(); });
                if (levelTransitionCooldown <= 0.0f && onGate && !anyEnemyAlive) {
                    // Simulate leaving the map through the gate: reset to center of the same map and respawn enemies
                    spdlog::info("Gate passed on tileID {} at (x={} y={}). Resetting to center and respawning enemies on same map.", gid, playerTileX, playerTileY);

                    // Update projection (same map, but keep consistent)
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();
                    float mapWidth = tilemap->getWidthInTiles() * tilemap->getTileWidth();
                    float mapHeight = tilemap->getHeightInTiles() * tilemap->getTileHeight();
                    glOrtho(0.0, mapWidth, mapHeight, 0.0, -1.0, 1.0);
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();

                    // Clear projectiles
                    playerProjectiles.clear();
                    enemyProjectiles.clear();

                    // Clear blood effects
                    for (auto& bloodEffect : bloodEffects) {
                        delete bloodEffect;
                    }
                    bloodEffects.clear();

                    // Respawn enemies: delete existing and recreate defaults
                    for (auto& enemy : enemies) {
                        if (enemy) {
                            delete enemy;
                        }
                    }
                    enemies.clear();
                    {
                        Enemy* flyingEye = new Enemy(25 * 16.0f, 10 * 16.0f, EnemyType::FlyingEye);
                        stbi_set_flip_vertically_on_load(true);
                        flyingEye->loadTexture(initializer.getAssetPath("assets/graphic/enemies/flying_eye/flgyingeye.png"), 150, 150, 8);
                        flyingEye->loadHitTexture(initializer.getAssetPath("assets/graphic/enemies/flying_eye/Hit_eye.png"), 150, 150, 4);
                        flyingEye->loadDeathTexture(initializer.getAssetPath("assets/graphic/enemies/flying_eye/Death_eye.png"), 150, 150, 4);
                        stbi_set_flip_vertically_on_load(false);
                        enemies.push_back(flyingEye);
                    }
                    {
                        Enemy* shroom = new Enemy(15 * 16.0f, 12 * 16.0f, EnemyType::Shroom);
                        stbi_set_flip_vertically_on_load(true);
                        shroom->loadTexture(initializer.getAssetPath("assets/graphic/enemies/shroom/shroom.png"), 150, 150, 8);
                        shroom->loadHitTexture(initializer.getAssetPath("assets/graphic/enemies/shroom/Hit_shroom.png"), 150, 150, 4);
                        shroom->loadDeathTexture(initializer.getAssetPath("assets/graphic/enemies/shroom/Death_shroom.png"), 150, 150, 4);
                        stbi_set_flip_vertically_on_load(false);
                        enemies.push_back(shroom);
                    }

                    // Teleport player to map center
                    float centerX = mapWidth * 0.5f;
                    float centerY = mapHeight * 0.5f;
                    float dx = centerX - player->getX();
                    float dy = centerY - player->getY();
                    player->move(dx, dy);

                    // Regenerate player's HP to full on gate entry
                    int healAmount = player->getMaxHealth() - player->getCurrentHealth();
                    if (healAmount > 0) {
                        player->heal(healAmount);
                    }

                    // Prevent immediate retriggering
                    levelTransitionCooldown = 0.5f;
                }
            }
            
            // Play shoot sound if player shot
            if (playerProjectiles.size() > 0 && playerProjectiles.back().isActive()) {
                // audioManager->playSound("shoot", 0.7f); // Commented out - no shoot sound available
            }
        
            tilemap->draw();
            
            // Update and draw blood effects (ground layer)
            spdlog::info("Drawing {} blood effects", bloodEffects.size());
            for (auto& bloodEffect : bloodEffects) {
                if (bloodEffect) {
                    bloodEffect->update(deltaTime);
                    bloodEffect->draw();
                    spdlog::info("Drew blood effect at ({}, {})", bloodEffect->getX(), bloodEffect->getY());
                }
            }
            
            player->draw();
            
            // Update and draw enemy
            for (auto& enemy : enemies) {
                enemy->update(deltaTime, player->getX(), player->getY(), *tilemap);
                enemy->updateAnimation(deltaTime);
                enemy->draw();
            }
            
            // Handle player-enemy collisions
            collisionManager.handlePlayerEnemyCollisions(player, enemies);
            
            // Handle enemy-to-enemy collisions
            collisionManager.handleEnemyEnemyCollisions(enemies);
            
            // Enemy shooting
            for (auto& enemy : enemies) {
                enemy->shootProjectile(player->getX(), player->getY(), enemyProjectiles);
            }
            
            // Update and draw projectiles
            for (auto& projectile : playerProjectiles) {
                projectile.update(deltaTime);
                projectile.draw();
            }
            
            for (auto& projectile : enemyProjectiles) {
                projectile.update(deltaTime);
                projectile.draw();
            }
            
            // Handle projectile-wall collisions
            collisionManager.handleProjectileWallCollisions(playerProjectiles, enemyProjectiles, *tilemap);
            
            // Handle projectile-entity collisions
            collisionManager.handleProjectileCollisions(playerProjectiles, enemyProjectiles, player, enemies);
            
            // Blood effect creation when enemy dies
            // Play blood effect and remove enemies after death delay
            spdlog::info("Checking {} enemies for blood effect creation", enemies.size());
            for (auto& enemy : enemies) {
                if (enemy) {
                    spdlog::info("Enemy at ({}, {}) - Alive: {}, Should create blood: {}", 
                                 enemy->getX(), enemy->getY(), enemy->isAlive(), enemy->shouldCreateBloodEffect());
                    if (!enemy->isAlive() && !enemy->shouldCreateBloodEffect()) {
                        spdlog::info("Enemy at ({}, {}) is dead but blood effect already created", enemy->getX(), enemy->getY());
                    }
                    if (enemy->shouldCreateBloodEffect()) {
                        bloodEffects.push_back(new BloodEffect(enemy->getX(), enemy->getY() + 12, initializer.getAssetPath(""))); // Move blood 12px down
                        enemy->markBloodEffectCreated();
                        // audioManager->playSound("enemy_death", 1.0f);
                        spdlog::info("Blood effect created at enemy death position ({}, {})", enemy->getX(), enemy->getY());
                    }
                }
            }
            // Remove enemies whose death timer has expired
            enemies.erase(
                std::remove_if(enemies.begin(), enemies.end(), [](Enemy* enemy) {
                    if (enemy->shouldRemoveAfterDeath()) {
                        delete enemy;
                        return true;
                    }
                    return false;
                }),
                enemies.end()
            );
            
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
            
            // Draw UI (player health bar, XP bar, and level indicator) LAST so it's always on top
            UI::drawPlayerHealth(player->getCurrentHealth(), player->getMaxHealth(), windowWidth, windowHeight);
            UI::drawXPBar(player->getCurrentXP(), player->getMaxXP(), windowWidth, windowHeight);
            UI::drawLevelIndicator(player->getLevel(), windowWidth, windowHeight);

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
            if (!player->isAlive()) {
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
            if (gameInitialized) {
                tilemap->draw();
                
                // Draw blood effects (ground layer) - NO UPDATE, just draw
                for (auto& bloodEffect : bloodEffects) {
                    bloodEffect->draw();
                }
                
                player->draw();
                
                // Draw enemies - NO UPDATE, just draw
                for (auto& enemy : enemies) {
                    enemy->draw();
                }
                
                // Draw projectiles - NO UPDATE, just draw
                for (auto& projectile : playerProjectiles) {
                    projectile.draw();
                }
                
                for (auto& projectile : enemyProjectiles) {
                    projectile.draw();
                }
                
                // Draw UI (player health bar, XP bar, and level indicator)
                UI::drawPlayerHealth(player->getCurrentHealth(), player->getMaxHealth(), windowWidth, windowHeight);
                UI::drawXPBar(player->getCurrentXP(), player->getMaxXP(), windowWidth, windowHeight);
                UI::drawLevelIndicator(player->getLevel(), windowWidth, windowHeight);
            }
            
            // Draw pause menu overlay
            UI::drawPauseScreen(windowWidth, windowHeight, selectedPauseButton);
        }
        else if (currentState == GameState::SAVE_SLOT_SELECTION) {
            // Draw game in background (paused)
            if (tilemap) {
                tilemap->draw();
            }
            if (player) {
                player->draw();
            }
            for (const auto& enemy : enemies) {
                if (enemy) {
                    enemy->draw();
                }
            }
            for (const auto& projectile : playerProjectiles) {
                projectile.draw();
            }
            for (const auto& projectile : enemyProjectiles) {
                projectile.draw();
            }
            for (const auto& bloodEffect : bloodEffects) {
                if (bloodEffect) {
                    bloodEffect->draw();
                }
            }
            
            // Draw UI elements
            if (player) {
                UI::drawPlayerHealth(player->getCurrentHealth(), player->getMaxHealth(), windowWidth, windowHeight);
                UI::drawXPBar(player->getCurrentXP(), player->getMaxXP(), windowWidth, windowHeight);
                UI::drawLevelIndicator(player->getLevel(), windowWidth, windowHeight);
            }
            
            // Draw save slot selection menu
            UI::drawSaveSlotMenu(windowWidth, windowHeight, selectedSaveSlot, saveSlotInfo);
        }
        else if (currentState == GameState::LOAD_SLOT_SELECTION) {
            // Draw game in background (paused)
            if (tilemap) {
                tilemap->draw();
            }
            if (player) {
                player->draw();
            }
            for (const auto& enemy : enemies) {
                if (enemy) {
                    enemy->draw();
                }
            }
            for (const auto& projectile : playerProjectiles) {
                projectile.draw();
            }
            for (const auto& projectile : enemyProjectiles) {
                projectile.draw();
            }
            for (const auto& bloodEffect : bloodEffects) {
                if (bloodEffect) {
                    bloodEffect->draw();
                }
            }
            
            // Draw UI elements
            if (player) {
                UI::drawPlayerHealth(player->getCurrentHealth(), player->getMaxHealth(), windowWidth, windowHeight);
                UI::drawXPBar(player->getCurrentXP(), player->getMaxXP(), windowWidth, windowHeight);
                UI::drawLevelIndicator(player->getLevel(), windowWidth, windowHeight);
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
                    if (gameInitialized && player) {
                        SaveData saveData = GameStateManager::createSaveData(player, enemies, playerProjectiles, enemyProjectiles, currentLevelPath, levelTransitionCooldown);
                        
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
                        
                        // Load the game state
                        spdlog::info("=== MAIN: About to call GameStateManager::loadGameState ===");
                        GameStateManager::loadGameState(saveData, player, enemies, playerProjectiles, enemyProjectiles, currentLevelPath, levelTransitionCooldown, initializer.getAssetPath(""));
                        spdlog::info("=== MAIN: GameStateManager::loadGameState completed ===");
                        
                        // Reload the tilemap for the saved level
                        if (tilemap) {
                            delete tilemap;
                        }
                        tilemap = new Tilemap();
                        if (!tilemap->loadTilesetTexture(initializer.getAssetPath("assets/graphic/tileset/tileset.png"), 16, 16)) {
                            spdlog::error("Failed to load tileset texture");
                            return -1;
                        }
                        if (!tilemap->loadFromJSON(currentLevelPath)) {
                            spdlog::error("Failed to load tilemap for saved level: {}", currentLevelPath);
                            // Fallback to default level
                            tilemap->loadFromJSON(initializer.getAssetPath("assets/levels/level1.json"));
                            currentLevelPath = initializer.getAssetPath("assets/levels/level1.json");
                        }
                        
                        // Set up projection to match tilemap size
                        glMatrixMode(GL_PROJECTION);
                        glLoadIdentity();
                        float mapWidth = tilemap->getWidthInTiles() * tilemap->getTileWidth();
                        float mapHeight = tilemap->getHeightInTiles() * tilemap->getTileHeight();
                        glOrtho(0.0, mapWidth, mapHeight, 0.0, -1.0, 1.0);
                        glMatrixMode(GL_MODELVIEW);
                        glLoadIdentity();
                        
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
                    if (gameInitialized) {
                        delete player;
                        player = nullptr;
                        for (auto& enemy : enemies) {
                            delete enemy;
                        }
                        enemies.clear();
                        delete inputHandler;
                        inputHandler = nullptr;
                        delete tilemap;
                        tilemap = nullptr;
                        gameInitialized = false;
                    }
                    playerProjectiles.clear();
                    enemyProjectiles.clear();
                    // Keep blood effects - don't clear them on respawn
                    // Blood effects should persist to show battle history
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
                    if (gameInitialized) {
                        delete player;
                        player = nullptr;
                        for (auto& enemy : enemies) {
                            delete enemy;
                        }
                        enemies.clear();
                        delete inputHandler;
                        inputHandler = nullptr;
                        delete tilemap;
                        tilemap = nullptr;
                        gameInitialized = false;
                    }
                    playerProjectiles.clear();
                    enemyProjectiles.clear();
                    // Keep blood effects - don't clear them on respawn
                    // Blood effects should persist to show battle history
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

    // Cleanup
    if (gameInitialized) {
        delete player;
        for (auto& enemy : enemies) {
            delete enemy;
        }
        enemies.clear();
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
    
    // Cleanup projectile texture
    Projectile::cleanupProjectileTexture();

    spdlog::info("Shutting down Ortos II application");
    // GameInitializer will handle cleanup in its destructor
    return 0;
}