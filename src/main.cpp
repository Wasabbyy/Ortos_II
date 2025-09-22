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
#include <iostream>
#include "nlohmann/json.hpp"
#include <stb_image.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <ctime>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
using json = nlohmann::json;

// Function to get the correct asset path regardless of where the executable is run from
std::string getAssetPath(const std::string& relativePath) {
    // Get the directory where the executable is located
    std::filesystem::path exePath = std::filesystem::current_path();
    
    // Try to find the project root by looking for the assets directory
    std::filesystem::path currentPath = exePath;
    
    // Check if we're already in the project root (assets directory exists)
    if (std::filesystem::exists(currentPath / "assets")) {
        return (currentPath / relativePath).string();
    }
    
    // If we're in the build directory, go up one level
    if (currentPath.filename() == "build") {
        currentPath = currentPath.parent_path();
        if (std::filesystem::exists(currentPath / "assets")) {
            return (currentPath / relativePath).string();
        }
    }
    
    // Try going up directories to find the project root
    for (int i = 0; i < 10; i++) {
        currentPath = currentPath.parent_path();
        if (std::filesystem::exists(currentPath / "assets")) {
            return (currentPath / relativePath).string();
        }
    }
    
    // If we still can't find it, try to find the executable's directory
    // This handles the case where the executable is run from anywhere
    std::filesystem::path executablePath;
    
#ifdef __APPLE__
    // macOS-specific executable path detection
    char buffer[1024];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        executablePath = std::filesystem::path(buffer).parent_path();
    }
#else
    // Linux-specific executable path detection
    try {
        executablePath = std::filesystem::canonical("/proc/self/exe");
    } catch (const std::filesystem::filesystem_error& e) {
        // Fallback if /proc/self/exe is not available
        executablePath = std::filesystem::path();
    }
#endif
    
    if (!executablePath.empty()) {
        // If executable is in build directory, go up one level
        if (executablePath.filename() == "build") {
            executablePath = executablePath.parent_path();
        }
        
        // Check if assets directory exists relative to executable
        if (std::filesystem::exists(executablePath / "assets")) {
            return (executablePath / relativePath).string();
        }
        
        // Try going up from executable directory
        for (int i = 0; i < 5; i++) {
            executablePath = executablePath.parent_path();
            if (std::filesystem::exists(executablePath / "assets")) {
                return (executablePath / relativePath).string();
            }
        }
    }
    
    // Last resort: try common project locations
    std::vector<std::string> commonPaths = {
        "/Users/filipstupar/Documents/OrtosII",
        "./",
        "../",
        "../../",
        "../../../"
    };
    
    for (const auto& path : commonPaths) {
        std::filesystem::path testPath = std::filesystem::absolute(path);
        if (std::filesystem::exists(testPath / "assets")) {
            return (testPath / relativePath).string();
        }
    }
    
    // If we can't find the assets directory, return the original path
    // This will cause an error, but it's better than crashing
    spdlog::warn("Could not find assets directory, using relative path: {}", relativePath);
    return relativePath;
}

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    DEATH,
    SAVE_SLOT_SELECTION,
    LOAD_SLOT_SELECTION
};

// Save data structure
struct SaveData {
    // Player data
    float playerX;
    float playerY;
    int playerHealth;
    int playerMaxHealth;
    int playerXP;
    int playerMaxXP;
    int playerLevel;
    
    // Enemy data
    std::vector<json> enemies;
    
    // Projectile data
    std::vector<json> playerProjectiles;
    std::vector<json> enemyProjectiles;
    
    // Game state
    std::string currentLevelPath;
    float levelTransitionCooldown;
    
    // Timestamp
    std::string saveTime;
};

// Save slot information
struct SaveSlot {
    int slotNumber;
    std::string filename;
    std::string saveTime;
    bool exists;
    SaveData data;
};

// Multiple save slots system
const int MAX_SAVE_SLOTS = 3;
std::vector<SaveSlot> saveSlots(MAX_SAVE_SLOTS);


// Initialize save slots
void initializeSaveSlots() {
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        saveSlots[i].slotNumber = i + 1;
        saveSlots[i].filename = getAssetPath("saves/savegame_slot" + std::to_string(i + 1) + ".json");
        saveSlots[i].exists = false;
        saveSlots[i].saveTime = "";
    }
}

// Check which save slots exist
void updateSaveSlots() {
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        std::ifstream file(saveSlots[i].filename);
        saveSlots[i].exists = file.good();
        file.close();
        
        if (saveSlots[i].exists) {
            // Load save time from file
            try {
                std::ifstream inFile(saveSlots[i].filename);
                json saveJson;
                inFile >> saveJson;
                inFile.close();
                saveSlots[i].saveTime = saveJson["gameState"]["saveTime"];
            } catch (...) {
                saveSlots[i].saveTime = "Unknown";
            }
        }
    }
}

// Get the most recent save slot (by save time)
int getMostRecentSaveSlot() {
    int mostRecentSlot = -1;
    std::string mostRecentTime = "";
    
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        if (saveSlots[i].exists && saveSlots[i].saveTime > mostRecentTime) {
            mostRecentTime = saveSlots[i].saveTime;
            mostRecentSlot = i;
        }
    }
    
    return mostRecentSlot;
}


// Save game function
bool saveGame(const SaveData& saveData, const std::string& filename = "savegame.json") {
    try {
        json saveJson;
        saveJson["player"]["x"] = saveData.playerX;
        saveJson["player"]["y"] = saveData.playerY;
        saveJson["player"]["health"] = saveData.playerHealth;
        saveJson["player"]["maxHealth"] = saveData.playerMaxHealth;
        saveJson["player"]["xp"] = saveData.playerXP;
        saveJson["player"]["maxXP"] = saveData.playerMaxXP;
        saveJson["player"]["level"] = saveData.playerLevel;
        
        saveJson["enemies"] = saveData.enemies;
        saveJson["playerProjectiles"] = saveData.playerProjectiles;
        saveJson["enemyProjectiles"] = saveData.enemyProjectiles;
        
        saveJson["gameState"]["currentLevelPath"] = saveData.currentLevelPath;
        saveJson["gameState"]["levelTransitionCooldown"] = saveData.levelTransitionCooldown;
        saveJson["gameState"]["saveTime"] = saveData.saveTime;
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            spdlog::error("Failed to open save file: {}", filename);
            return false;
        }
        
        file << saveJson.dump(4); // Pretty print with 4 spaces
        file.close();
        
        spdlog::info("Game saved successfully to: {}", filename);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to save game: {}", e.what());
        return false;
    }
}

// Load game function
bool loadGame(SaveData& saveData, const std::string& filename = "savegame.json") {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            spdlog::error("Save file not found: {}", filename);
            return false;
        }
        
        json saveJson;
        file >> saveJson;
        file.close();
        
        // Load player data
        saveData.playerX = saveJson["player"]["x"];
        saveData.playerY = saveJson["player"]["y"];
        saveData.playerHealth = saveJson["player"]["health"];
        saveData.playerMaxHealth = saveJson["player"]["maxHealth"];
        saveData.playerXP = saveJson["player"]["xp"];
        saveData.playerMaxXP = saveJson["player"]["maxXP"];
        saveData.playerLevel = saveJson["player"]["level"];
        
        // Load enemy data
        saveData.enemies = saveJson["enemies"];
        saveData.playerProjectiles = saveJson["playerProjectiles"];
        saveData.enemyProjectiles = saveJson["enemyProjectiles"];
        
        // Load game state
        saveData.currentLevelPath = saveJson["gameState"]["currentLevelPath"];
        saveData.levelTransitionCooldown = saveJson["gameState"]["levelTransitionCooldown"];
        saveData.saveTime = saveJson["gameState"]["saveTime"];
        
        spdlog::info("Game loaded successfully from: {}", filename);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to load game: {}", e.what());
        return false;
    }
}

// Check if save file exists
bool saveFileExists(const std::string& filename = "savegame.json") {
    std::ifstream file(filename);
    return file.good();
}

// Load game state from save data
void loadGameState(const SaveData& saveData, Player*& player, std::vector<Enemy*>& enemies, 
                   std::vector<Projectile>& playerProjectiles, std::vector<Projectile>& enemyProjectiles,
                   std::string& currentLevelPath, float& levelTransitionCooldown) {
    if (!player) return;
    
    // Restore player state - we need to create a new player with the saved state
    // since the Player class doesn't have setter methods
    delete player;
    player = new Player();
    
    // Load player textures
    stbi_set_flip_vertically_on_load(true);
    player->loadTexture(getAssetPath("assets/graphic/enemies/vampire/Vampire_Walk.png"), 64, 64, 4);
    player->loadIdleTexture(getAssetPath("assets/graphic/enemies/vampire/Vampire_Idle.png"), 64, 64, 2);
    stbi_set_flip_vertically_on_load(false);
    
    // Move player to saved position
    player->move(saveData.playerX - player->getX(), saveData.playerY - player->getY());
    
    // Restore health by healing/damaging as needed
    int healthDiff = saveData.playerHealth - player->getCurrentHealth();
    if (healthDiff > 0) {
        player->heal(healthDiff);
    } else if (healthDiff < 0) {
        player->takeDamage(-healthDiff);
    }
    
    // Restore XP by gaining XP
    int xpDiff = saveData.playerXP - player->getCurrentXP();
    if (xpDiff > 0) {
        player->gainXP(xpDiff);
    }
    
    // Restore game state
    currentLevelPath = saveData.currentLevelPath;
    levelTransitionCooldown = saveData.levelTransitionCooldown;
    
    // Clear existing projectiles
    playerProjectiles.clear();
    enemyProjectiles.clear();
    
    // Clear existing enemies
    for (auto& enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
    
    // Restore enemies from save data
    for (const auto& enemyData : saveData.enemies) {
        // Create enemy based on saved data
        EnemyType enemyType = static_cast<EnemyType>(enemyData["type"]);
        Enemy* enemy = new Enemy(enemyData["x"], enemyData["y"], enemyType);
        
        // Load appropriate textures based on enemy type
        stbi_set_flip_vertically_on_load(true);
        if (enemyType == EnemyType::FlyingEye) {
            enemy->loadTexture(getAssetPath("assets/graphic/enemies/flying_eye/flgyingeye.png"), 150, 150, 8);
            enemy->loadHitTexture(getAssetPath("assets/graphic/enemies/flying_eye/Hit_eye.png"), 150, 150, 4);
            enemy->loadDeathTexture(getAssetPath("assets/graphic/enemies/flying_eye/Death_eye.png"), 150, 150, 4);
        } else if (enemyType == EnemyType::Shroom) {
            enemy->loadTexture(getAssetPath("assets/graphic/enemies/shroom/shroom.png"), 150, 150, 8);
            enemy->loadHitTexture(getAssetPath("assets/graphic/enemies/shroom/Hit_shroom.png"), 150, 150, 4);
            enemy->loadDeathTexture(getAssetPath("assets/graphic/enemies/shroom/Death_shroom.png"), 150, 150, 4);
        }
        stbi_set_flip_vertically_on_load(false);
        
        // Restore enemy state
        enemy->setAlive(enemyData["alive"]);
        // Restore health by taking damage if needed
        int maxHealth = enemyData["maxHealth"].get<int>();
        int currentHealth = enemyData["health"].get<int>();
        int healthDiff = maxHealth - currentHealth;
        if (healthDiff > 0) {
            enemy->takeDamage(healthDiff);
        }
        
        enemies.push_back(enemy);
    }
    
    spdlog::info("Game state loaded successfully");
}

int main() {
    // Initialize logger (console + file). Truncate file on each start.
    try {
        std::filesystem::create_directories("logs");
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/ortos.log", true);
        std::vector<spdlog::sink_ptr> sinks { console_sink, file_sink };
        auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::info);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        spdlog::info("Starting Ortos II application");
    } catch (const std::exception& e) {
        // Fallback to default logger if file sink fails
        spdlog::set_level(spdlog::level::debug);
        spdlog::warn("Failed to initialize file logger: {}", e.what());
    }
    
    // Initialize save slots system
    initializeSaveSlots();
    updateSaveSlots();
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
    if (!UI::init(getAssetPath("assets/fonts/pixel.ttf"))) {
        spdlog::error("Failed to initialize UI system");
        glfwTerminate();
        return -1;
    }

    // Load title screen background texture
    if (!UI::loadTitleScreenTexture(getAssetPath("assets/screens/titlescreen.png"))) {
        spdlog::warn("Failed to load title screen texture, will use black background");
    }

    // Load death screen background texture
    if (!UI::loadDeathScreenTexture(getAssetPath("assets/screens/deathscreen.png"))) {
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
    if (!uiAudioManager.loadUISound("button", getAssetPath("assets/sounds/button.wav"))) {
        spdlog::warn("Failed to load button sound");
    } else {
        spdlog::info("Successfully loaded button sound");
    }

    // Load intro music for title screen
    spdlog::info("Attempting to load intro music...");
    if (!audioManager.loadMusic("intro", getAssetPath("assets/sounds/intro.wav"))) {
        spdlog::warn("Failed to load intro music");
    } else {
        spdlog::info("Successfully loaded intro music");
    }

    // Load background music for gameplay
    spdlog::info("Attempting to load background music...");
    if (!audioManager.loadMusic("background", getAssetPath("assets/sounds/defaultSong.wav"))) {
        spdlog::warn("Failed to load background music");
    } else {
        spdlog::info("Successfully loaded background music");
    }

    // Load all projectile textures (player, eye, shroom)
    Projectile::loadAllProjectileTextures();

    // Set up viewport and orthographic projection
    int windowWidth = 1920;
    int windowHeight = 1080;
    glfwSetWindowSize(window, windowWidth, windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

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
    std::vector<std::string> saveSlotInfo(3, "Empty");
    bool loadSlotFromMainMenu = false;
    // Level management
    std::string currentLevelPath = getAssetPath("assets/maps/test.json");
    std::string nextLevelPath = getAssetPath("assets/maps/final.json");
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
            updateSaveSlots();
            hasSaveFile = false;
            for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
                if (saveSlots[i].exists) {
                    hasSaveFile = true;
                    break;
                }
            }
            
            // Start intro music if not already started
            if (!introMusicStarted) {
                audioManager.playMusic("intro", true); // Loop the intro music
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
                uiAudioManager.playButtonHoverSound();
                    hoverSoundPlayed = true;
                }
                previousSelectedMenuOption = selectedMenuOption;
            } else {
                hoverSoundPlayed = false; // Reset when not changing
            }
            
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed) {
                    uiAudioManager.playButtonClickSound();
                
                if (hasSaveFile) {
                    // Menu with save file: Start Game, Continue Game, Load Game, Exit Game
                    if (selectedMenuOption == 0) {
                        // Start new game - reset game state
                        spdlog::info("Starting new game");
                        gameInitialized = false; // Force re-initialization
                        currentState = GameState::PLAYING;
                    } else if (selectedMenuOption == 1) {
                        // Load game - go to load slot selection
                        updateSaveSlots();
                        // Update save slot info for display
                        for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
                            if (saveSlots[i].exists) {
                                saveSlotInfo[i] = saveSlots[i].saveTime;
                            } else {
                                saveSlotInfo[i] = "Empty";
                            }
                        }
                        selectedSaveSlot = 0;
                        loadSlotMenuInitialized = false;
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
                    audioManager.stopMusic();
                    introMusicStarted = false;
                    // Set lower volume for background music during gameplay
                    audioManager.setMusicVolume(0.4f); // Reduced from default 1.0 to 0.4
                    // Start background music for gameplay
                    audioManager.playMusic("background", true); // Loop the background music
                    backgroundMusicStarted = true;
                    spdlog::info("Started background music for gameplay at reduced volume (0.4)");
                    
                    // Load game if continuing from save
                    if (hasSaveFile && (selectedMenuOption == 1 || selectedMenuOption == 2)) {
                        SaveData saveData;
                        if (loadGame(saveData)) {
                            // Initialize game objects first if not already done
                            if (!gameInitialized) {
                                // Initialize all game objects properly
                                player = new Player();
                                stbi_set_flip_vertically_on_load(true);
                                player->loadTexture(getAssetPath("assets/graphic/enemies/vampire/Vampire_Walk.png"), 64, 64, 4);
                                player->loadIdleTexture(getAssetPath("assets/graphic/enemies/vampire/Vampire_Idle.png"), 64, 64, 2);
                                stbi_set_flip_vertically_on_load(false);
                                
                                inputHandler = new InputHandler();
                                tilemap = new Tilemap();
                                if (!tilemap->loadTilesetTexture(getAssetPath("assets/graphic/tileset/tileset.png"), 16, 16)) {
                                    spdlog::error("Failed to load tileset texture");
                                    return -1;
                                }
                                
                                // Load projectile texture
                                Projectile::loadProjectileTexture(getAssetPath("assets/graphic/projectiles/green_projectiles.png"));
                                
                                gameInitialized = true;
                            }
                            // Load the game state
                            loadGameState(saveData, player, enemies, playerProjectiles, enemyProjectiles, currentLevelPath, levelTransitionCooldown);
                            
                            // Reload the tilemap for the saved level
                            if (tilemap) {
                                delete tilemap;
                            }
                            tilemap = new Tilemap();
                            if (!tilemap->loadTilesetTexture(getAssetPath("assets/graphic/tileset/tileset.png"), 16, 16)) {
                                spdlog::error("Failed to load tileset texture");
                                return -1;
                            }
                            if (!tilemap->loadFromJSON(currentLevelPath)) {
                                spdlog::error("Failed to load tilemap for saved level: {}", currentLevelPath);
                                // Fallback to default level
                                tilemap->loadFromJSON(getAssetPath("assets/levels/level1.json"));
                                currentLevelPath = getAssetPath("assets/levels/level1.json");
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
                // Clean up any existing game objects first
                if (player) {
                    delete player;
                    player = nullptr;
                }
                for (auto& enemy : enemies) {
                    delete enemy;
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
                
                player = new Player();
                stbi_set_flip_vertically_on_load(true);
                player->loadTexture(getAssetPath("assets/graphic/enemies/vampire/Vampire_Walk.png"), 64, 64, 4);
                player->loadIdleTexture(getAssetPath("assets/graphic/enemies/vampire/Vampire_Idle.png"), 64, 64, 2);
                stbi_set_flip_vertically_on_load(false);
                
                // Create enemies
                // Flying eye enemy
                Enemy* flyingEye = new Enemy(25 * 16.0f, 10 * 16.0f, EnemyType::FlyingEye);
                stbi_set_flip_vertically_on_load(true);
                flyingEye->loadTexture(getAssetPath("assets/graphic/enemies/flying_eye/flgyingeye.png"), 150, 150, 8);
                flyingEye->loadHitTexture(getAssetPath("assets/graphic/enemies/flying_eye/Hit_eye.png"), 150, 150, 4);
                flyingEye->loadDeathTexture(getAssetPath("assets/graphic/enemies/flying_eye/Death_eye.png"), 150, 150, 4); // NEW
                stbi_set_flip_vertically_on_load(false);
                enemies.push_back(flyingEye);
                
                // Shroom enemy
                Enemy* shroom = new Enemy(15 * 16.0f, 12 * 16.0f, EnemyType::Shroom);
                stbi_set_flip_vertically_on_load(true);
                shroom->loadTexture(getAssetPath("assets/graphic/enemies/shroom/shroom.png"), 150, 150, 8);
                shroom->loadHitTexture(getAssetPath("assets/graphic/enemies/shroom/Hit_shroom.png"), 150, 150, 4);
                shroom->loadDeathTexture(getAssetPath("assets/graphic/enemies/shroom/Death_shroom.png"), 150, 150, 4); // NEW
                stbi_set_flip_vertically_on_load(false);
                enemies.push_back(shroom);
                
                inputHandler = new InputHandler();
                tilemap = new Tilemap();
                if (!tilemap->loadTilesetTexture(getAssetPath("assets/graphic/tileset/tileset.png"), 16, 16)) {
                    spdlog::error("Failed to load tileset texture");
                    return -1;
                }
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
                Projectile::loadProjectileTexture(getAssetPath("assets/graphic/projectiles/green_projectiles.png"));
                
                gameInitialized = true;
                spdlog::info("Game initialized successfully");
                
                // Load sound effects (only load what's available)
                if (!audioManager.loadSound("intro", getAssetPath("assets/sounds/intro.wav"))) {
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
                        delete enemy;
                    }
                    enemies.clear();
                    {
                        Enemy* flyingEye = new Enemy(25 * 16.0f, 10 * 16.0f, EnemyType::FlyingEye);
                        stbi_set_flip_vertically_on_load(true);
                        flyingEye->loadTexture(getAssetPath("assets/graphic/enemies/flying_eye/flgyingeye.png"), 150, 150, 8);
                        flyingEye->loadHitTexture(getAssetPath("assets/graphic/enemies/flying_eye/Hit_eye.png"), 150, 150, 4);
                        flyingEye->loadDeathTexture(getAssetPath("assets/graphic/enemies/flying_eye/Death_eye.png"), 150, 150, 4);
                        stbi_set_flip_vertically_on_load(false);
                        enemies.push_back(flyingEye);
                    }
                    {
                        Enemy* shroom = new Enemy(15 * 16.0f, 12 * 16.0f, EnemyType::Shroom);
                        stbi_set_flip_vertically_on_load(true);
                        shroom->loadTexture(getAssetPath("assets/graphic/enemies/shroom/shroom.png"), 150, 150, 8);
                        shroom->loadHitTexture(getAssetPath("assets/graphic/enemies/shroom/Hit_shroom.png"), 150, 150, 4);
                        shroom->loadDeathTexture(getAssetPath("assets/graphic/enemies/shroom/Death_shroom.png"), 150, 150, 4);
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
            for (auto& enemy : enemies) {
                enemy->update(deltaTime, player->getX(), player->getY(), *tilemap);
                enemy->updateAnimation(deltaTime);
                enemy->draw();
            }
            
            // Player-Enemy collision detection (no pushing - just stop movement)
            bool playerCollidingWithEnemy = false; // Track if player is colliding with any enemy
            
            for (auto& enemy : enemies) {
                if (enemy->isAlive()) {
                    // Quick distance check first to avoid expensive collision calculations
                    float dx = player->getX() - enemy->getX();
                    float dy = player->getY() - enemy->getY();
                    float distanceSquared = dx * dx + dy * dy;
                    float maxDistance = 64.0f; // Only check collision if within reasonable distance
                    
                    if (distanceSquared < maxDistance * maxDistance) {
                        // Check if player and enemy bounding boxes overlap
                        bool collision = !(player->getRight() < enemy->getLeft() || 
                                         player->getLeft() > enemy->getRight() || 
                                         player->getBottom() < enemy->getTop() || 
                                         player->getTop() > enemy->getBottom());
                        
                        if (collision) {
                            playerCollidingWithEnemy = true; // Mark that player is colliding
                            
                            // Calculate overlap amounts
                            float overlapLeft = player->getRight() - enemy->getLeft();
                            float overlapRight = enemy->getRight() - player->getLeft();
                            float overlapTop = player->getBottom() - enemy->getTop();
                            float overlapBottom = enemy->getBottom() - player->getTop();
                            
                            // Find the minimum overlap to determine separation direction
                            float minOverlap = std::min({overlapLeft, overlapRight, overlapTop, overlapBottom});
                            
                            // Separate entities by moving them apart (no pushing, just separation)
                            float separationAmount = minOverlap * 0.5f; // Half overlap to each entity
                            if (minOverlap == overlapLeft) {
                                // Separate horizontally - move player left, enemy right
                                player->move(-separationAmount, 0);
                                enemy->move(separationAmount, 0);
                            } else if (minOverlap == overlapRight) {
                                // Separate horizontally - move player right, enemy left
                                player->move(separationAmount, 0);
                                enemy->move(-separationAmount, 0);
                            } else if (minOverlap == overlapTop) {
                                // Separate vertically - move player up, enemy down
                                player->move(0, -separationAmount);
                                enemy->move(0, separationAmount);
                            } else if (minOverlap == overlapBottom) {
                                // Separate vertically - move player down, enemy up
                                player->move(0, separationAmount);
                                enemy->move(0, -separationAmount);
                            }
                        }
                    }
                }
            }
            
            // Update player collision state
            player->setCollidingWithEnemy(playerCollidingWithEnemy);
            
            // Enemy-to-Enemy collision detection (no pushing - just separation)
            for (size_t i = 0; i < enemies.size(); ++i) {
                if (!enemies[i]->isAlive()) continue;
                
                for (size_t j = i + 1; j < enemies.size(); ++j) {
                    if (!enemies[j]->isAlive()) continue;
                    
                    // Quick distance check first
                    float dx = enemies[i]->getX() - enemies[j]->getX();
                    float dy = enemies[i]->getY() - enemies[j]->getY();
                    float distanceSquared = dx * dx + dy * dy;
                    float maxDistance = 64.0f;
                    
                    if (distanceSquared < maxDistance * maxDistance) {
                        // Check if enemy bounding boxes overlap
                        bool collision = !(enemies[i]->getRight() < enemies[j]->getLeft() || 
                                         enemies[i]->getLeft() > enemies[j]->getRight() || 
                                         enemies[i]->getBottom() < enemies[j]->getTop() || 
                                         enemies[i]->getTop() > enemies[j]->getBottom());
                        
                        if (collision) {
                            // Calculate overlap amounts
                            float overlapLeft = enemies[i]->getRight() - enemies[j]->getLeft();
                            float overlapRight = enemies[j]->getRight() - enemies[i]->getLeft();
                            float overlapTop = enemies[i]->getBottom() - enemies[j]->getTop();
                            float overlapBottom = enemies[j]->getBottom() - enemies[i]->getTop();
                            
                            // Find the minimum overlap to determine separation direction
                            float minOverlap = std::min({overlapLeft, overlapRight, overlapTop, overlapBottom});
                            
                            // Separate enemies by moving them apart (no pushing, just separation)
                            float separationAmount = minOverlap * 0.25f; // Quarter overlap to each enemy
                            if (minOverlap == overlapLeft) {
                                // Separate enemies horizontally
                                enemies[i]->move(-separationAmount, 0);
                                enemies[j]->move(separationAmount, 0);
                            } else if (minOverlap == overlapRight) {
                                // Separate enemies horizontally
                                enemies[i]->move(separationAmount, 0);
                                enemies[j]->move(-separationAmount, 0);
                            } else if (minOverlap == overlapTop) {
                                // Separate enemies vertically
                                enemies[i]->move(0, -separationAmount);
                                enemies[j]->move(0, separationAmount);
                            } else if (minOverlap == overlapBottom) {
                                // Separate enemies vertically
                                enemies[i]->move(0, separationAmount);
                                enemies[j]->move(0, -separationAmount);
                            }
                        }
                    }
                }
            }
            
            // Enemy shooting
            for (auto& enemy : enemies) {
                enemy->shootProjectile(player->getX(), player->getY(), enemyProjectiles);
            }
            
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
                for (auto& enemy : enemies) {
                    if (projectile.isActive() && enemy->isAlive()) {
                        if (projectile.checkCollision(enemy->getX(), enemy->getY(), 8.0f)) {
                            projectile.setActive(false);
                            enemy->takeDamage(25, player);  // Deal 25 damage and pass player for XP reward
                            // audioManager.playSound("enemy_hit", 0.8f);
                            spdlog::info("Enemy hit by player projectile! Enemy HP: {}/{}", 
                                        enemy->getCurrentHealth(), enemy->getMaxHealth());
                        }
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
            // Play blood effect and remove enemies after death delay
            for (auto& enemy : enemies) {
                if (enemy->shouldCreateBloodEffect()) {
                    bloodEffects.push_back(new BloodEffect(enemy->getX(), enemy->getY() + 12)); // Move blood 12px down
                    enemy->markBloodEffectCreated();
                    // audioManager.playSound("enemy_death", 1.0f);
                    spdlog::info("Blood effect created at enemy death position ({}, {})", enemy->getX(), enemy->getY());
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
                // audioManager.playSound("player_death", 1.0f);
                // Stop background music when player dies
                audioManager.stopMusic();
                backgroundMusicStarted = false;
                // Reset music volume to normal for intro music
                audioManager.setMusicVolume(1.0f);
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
                    uiAudioManager.playButtonHoverSound();
                    hoverSoundPlayed = true;
                }
                previousSelectedPauseButton = selectedPauseButton;
            } else {
                hoverSoundPlayed = false; // Reset when not changing
            }
            
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed) {
                uiAudioManager.playButtonClickSound();
                
                if (selectedPauseButton == 0) {
                    // Resume game
                    currentState = GameState::PLAYING;
                    spdlog::info("Resuming game");
                } else if (selectedPauseButton == 1) {
                    // Save game - go to save slot selection
                    updateSaveSlots();
                    // Update save slot info for display
                    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
                        if (saveSlots[i].exists) {
                            saveSlotInfo[i] = saveSlots[i].saveTime;
                        } else {
                            saveSlotInfo[i] = "Empty";
                        }
                    }
                    selectedSaveSlot = 0;
                    saveSlotMenuInitialized = false;
                    currentState = GameState::SAVE_SLOT_SELECTION;
                    spdlog::info("Entering save slot selection");
                } else if (selectedPauseButton == 2) {
                    // Back to main menu
                    // Stop background music and reset introMusicStarted flag
                    audioManager.stopMusic();
                    introMusicStarted = false;
                    backgroundMusicStarted = false;
                    // Reset music volume to normal for intro music
                    audioManager.setMusicVolume(1.0f);
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
                        SaveData saveData;
                        saveData.playerX = player->getX();
                        saveData.playerY = player->getY();
                        saveData.playerHealth = player->getCurrentHealth();
                        saveData.playerMaxHealth = player->getMaxHealth();
                        saveData.playerXP = player->getCurrentXP();
                        saveData.playerMaxXP = player->getMaxXP();
                        saveData.playerLevel = player->getLevel();
                        saveData.currentLevelPath = currentLevelPath;
                        saveData.levelTransitionCooldown = levelTransitionCooldown;
                        
                        // Save current enemies
                        for (const auto& enemy : enemies) {
                            json enemyData;
                            enemyData["x"] = enemy->getX();
                            enemyData["y"] = enemy->getY();
                            enemyData["health"] = enemy->getCurrentHealth();
                            enemyData["maxHealth"] = enemy->getMaxHealth();
                            enemyData["alive"] = enemy->isAlive();
                            enemyData["type"] = static_cast<int>(enemy->getType());
                            enemyData["state"] = static_cast<int>(enemy->getState());
                            saveData.enemies.push_back(enemyData);
                        }
                        
                        // Get current time
                        auto now = std::chrono::system_clock::now();
                        auto time_t = std::chrono::system_clock::to_time_t(now);
                        saveData.saveTime = std::ctime(&time_t);
                        saveData.saveTime.pop_back(); // Remove newline
                        
                        if (saveGame(saveData, saveSlots[selectedSaveSlot].filename)) {
                            updateSaveSlots();
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
                if (selectedSaveSlot < 3 && saveSlots[selectedSaveSlot].exists) {
                    // Load from selected slot
                    SaveData saveData;
                    if (loadGame(saveData, saveSlots[selectedSaveSlot].filename)) {
                        
                        // Load the game state
                        loadGameState(saveData, player, enemies, playerProjectiles, enemyProjectiles, currentLevelPath, levelTransitionCooldown);
                        
                        // Reload the tilemap for the saved level
                        if (tilemap) {
                            delete tilemap;
                        }
                        tilemap = new Tilemap();
                        if (!tilemap->loadTilesetTexture(getAssetPath("assets/graphic/tileset/tileset.png"), 16, 16)) {
                            spdlog::error("Failed to load tileset texture");
                            return -1;
                        }
                        if (!tilemap->loadFromJSON(currentLevelPath)) {
                            spdlog::error("Failed to load tilemap for saved level: {}", currentLevelPath);
                            // Fallback to default level
                            tilemap->loadFromJSON(getAssetPath("assets/levels/level1.json"));
                            currentLevelPath = getAssetPath("assets/levels/level1.json");
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
                uiAudioManager.playButtonHoverSound();
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
                    uiAudioManager.playButtonClickSound();
                    // Reset game state
                    spdlog::info("Respawn button clicked, restarting game");
                    if (gameInitialized) {
                        delete player;
                        for (auto& enemy : enemies) {
                            delete enemy;
                        }
                        enemies.clear();
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
                    audioManager.setMusicVolume(0.4f); // Set lower volume for background music
                    audioManager.playMusic("background", true); // Loop the background music
                    backgroundMusicStarted = true;
                    spdlog::info("Started background music for gameplay at reduced volume (0.4)");
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
                        for (auto& enemy : enemies) {
                            delete enemy;
                        }
                        enemies.clear();
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
                    audioManager.setMusicVolume(0.4f); // Set lower volume for background music
                    audioManager.playMusic("background", true); // Loop the background music
                    backgroundMusicStarted = true;
                    spdlog::info("Started background music for gameplay at reduced volume (0.4)");
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
    glfwTerminate();
    return 0;
}