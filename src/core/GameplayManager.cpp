#include "GameplayManager.h"
#include <stb_image.h>
#include <spdlog/spdlog.h>
#include <algorithm>

GameplayManager::GameplayManager() 
    : player(nullptr)
    , inputHandler(nullptr)
    , tilemap(nullptr)
    , audioManager(nullptr)
    , uiAudioManager(nullptr)
    , saveManager(nullptr)
    , gameInitialized(false)
    , currentLevelPath("")
    , nextLevelPath("")
    , levelTransitionCooldown(0.0f)
    , assetPath("")
{
}

GameplayManager::~GameplayManager() {
    cleanup();
}

bool GameplayManager::initialize(const std::string& assetPath, AudioManager* audioManager, UIAudioManager* uiAudioManager) {
    this->assetPath = assetPath;
    this->audioManager = audioManager;
    this->uiAudioManager = uiAudioManager;
    
    // Set default level paths
    currentLevelPath = assetPath + "assets/maps/test.json";
    nextLevelPath = assetPath + "assets/maps/final.json";
    
    spdlog::info("GameplayManager initialized with asset path: {}", assetPath);
    return true;
}

void GameplayManager::setSaveManager(EnhancedSaveManager* saveManager) {
    this->saveManager = saveManager;
}

void GameplayManager::cleanup() {
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
    
    for (auto& bloodEffect : bloodEffects) {
        if (bloodEffect) {
            delete bloodEffect;
        }
    }
    bloodEffects.clear();
    
    playerProjectiles.clear();
    enemyProjectiles.clear();
    
    gameInitialized = false;
    spdlog::info("GameplayManager cleaned up");
}

void GameplayManager::startNewGame() {
    spdlog::info("Starting new game");
    resetGame();
    initializeGameObjects();
    loadLevel(currentLevelPath);
    
    // Create temporary player in database
    if (saveManager && saveManager->isDatabaseEnabled()) {
        SaveData tempSaveData = createSaveData();
        if (saveManager->createTemporaryPlayer(tempSaveData)) {
            spdlog::info("Temporary player created for new game");
        } else {
            spdlog::warn("Failed to create temporary player");
        }
    }
    
    gameInitialized = true;
    spdlog::info("New game started successfully");
}

void GameplayManager::loadGame(SaveData& saveData, const std::string& assetPath) {
    spdlog::info("Loading game state");
    
    // Initialize game objects first if not already done
    if (!gameInitialized) {
        initializeGameObjects();
        gameInitialized = true;
    }
    
    // Load the game state
    spdlog::info("=== GAMEPLAYMANAGER: About to call GameStateManager::loadGameState ===");
    GameStateManager::loadGameState(saveData, player, enemies, playerProjectiles, enemyProjectiles, currentLevelPath, levelTransitionCooldown, assetPath);
    spdlog::info("=== GAMEPLAYMANAGER: GameStateManager::loadGameState completed ===");
    
    // Reload the tilemap for the saved level
    if (tilemap) {
        delete tilemap;
    }
    tilemap = new Tilemap();
    if (!tilemap->loadTilesetTexture(assetPath + "assets/graphic/tileset/tileset.png", 16, 16)) {
        spdlog::error("Failed to load tileset texture");
        return;
    }
    if (!tilemap->loadFromJSON(currentLevelPath)) {
        spdlog::error("Failed to load tilemap for saved level: {}", currentLevelPath);
        // Fallback to default level
        tilemap->loadFromJSON(assetPath + "assets/levels/level1.json");
        currentLevelPath = assetPath + "assets/levels/level1.json";
    }
    
    setupProjection();
    spdlog::info("Game loaded successfully");
}

void GameplayManager::resetGame() {
    cleanup();
    gameInitialized = false;
}

void GameplayManager::update(float deltaTime, GLFWwindow* window, int windowWidth, int windowHeight) {
    if (!gameInitialized) return;
    
    updateGameLogic(deltaTime, window);
    updateEntities(deltaTime);
    handleCollisions();
    createBloodEffects();
    cleanupInactiveObjects();
}

void GameplayManager::draw(int windowWidth, int windowHeight) {
    if (!gameInitialized) return;
    
    drawGameWorld();
    drawEntities();
    drawProjectiles();
    drawBloodEffects();
    drawUI(windowWidth, windowHeight);
}

void GameplayManager::drawPaused(int windowWidth, int windowHeight) {
    if (!gameInitialized) return;
    
    // Draw the game in the background (paused state) - NO UPDATES, just drawing
    drawGameWorld();
    drawEntities();
    drawProjectiles();
    drawBloodEffects();
    drawUI(windowWidth, windowHeight);
}

SaveData GameplayManager::createSaveData() const {
    if (!gameInitialized) {
        return SaveData();
    }
    return GameStateManager::createSaveData(player, enemies, playerProjectiles, enemyProjectiles, currentLevelPath, levelTransitionCooldown);
}

void GameplayManager::loadGameState(const SaveData& saveData, const std::string& assetPath) {
    GameStateManager::loadGameState(saveData, player, enemies, playerProjectiles, enemyProjectiles, currentLevelPath, levelTransitionCooldown, assetPath);
}

void GameplayManager::updatePlayerStatsInDatabase() {
    if (!saveManager || !saveManager->isDatabaseEnabled() || !player) {
        return;
    }
    
    // Only update database if current player is temporary
    if (saveManager->isCurrentPlayerTemporary()) {
        SaveData currentSaveData = createSaveData();
        saveManager->updateTemporaryPlayerStats(currentSaveData);
    } else {
        spdlog::info("Skipping database update - player is permanent");
    }
}

void GameplayManager::initializeGameObjects() {
    spdlog::info("Initializing game objects...");
    
    // Clean up any existing game objects first
    cleanup();
    
    spdlog::info("Creating player...");
    player = new Player();
    stbi_set_flip_vertically_on_load(true);
    spdlog::info("Loading player textures...");
    player->loadTexture(assetPath + "assets/graphic/enemies/vampire/Vampire_Walk.png", 64, 64, 4);
    player->loadIdleTexture(assetPath + "assets/graphic/enemies/vampire/Vampire_Idle.png", 64, 64, 2);
    stbi_set_flip_vertically_on_load(false);
    
    // Create enemies
    createDefaultEnemies();
    
    spdlog::info("Creating input handler and tilemap...");
    inputHandler = new InputHandler();
    tilemap = new Tilemap();
    spdlog::info("Loading tileset texture...");
    if (!tilemap->loadTilesetTexture(assetPath + "assets/graphic/tileset/tileset.png", 16, 16)) {
        spdlog::error("Failed to load tileset texture");
        return;
    }
    
    // Load projectile texture
    spdlog::info("Loading projectile textures...");
    Projectile::loadProjectileTexture(assetPath + "assets/graphic/projectiles/green_projectiles.png");
    
    spdlog::info("Game objects initialized successfully");
}

void GameplayManager::createDefaultEnemies() {
    spdlog::info("Creating enemies...");
    
    // Flying eye enemy
    Enemy* flyingEye = new Enemy(25 * 16.0f, 10 * 16.0f, EnemyType::FlyingEye);
    stbi_set_flip_vertically_on_load(true);
    spdlog::info("Loading flying eye textures...");
    flyingEye->loadTexture(assetPath + "assets/graphic/enemies/flying_eye/flgyingeye.png", 150, 150, 8);
    flyingEye->loadHitTexture(assetPath + "assets/graphic/enemies/flying_eye/Hit_eye.png", 150, 150, 4);
    flyingEye->loadDeathTexture(assetPath + "assets/graphic/enemies/flying_eye/Death_eye.png", 150, 150, 4);
    stbi_set_flip_vertically_on_load(false);
    enemies.push_back(flyingEye);
    
    // Shroom enemy
    Enemy* shroom = new Enemy(15 * 16.0f, 12 * 16.0f, EnemyType::Shroom);
    stbi_set_flip_vertically_on_load(true);
    spdlog::info("Loading shroom textures...");
    shroom->loadTexture(assetPath + "assets/graphic/enemies/shroom/shroom.png", 150, 150, 8);
    shroom->loadHitTexture(assetPath + "assets/graphic/enemies/shroom/Hit_shroom.png", 150, 150, 4);
    shroom->loadDeathTexture(assetPath + "assets/graphic/enemies/shroom/Death_shroom.png", 150, 150, 4);
    stbi_set_flip_vertically_on_load(false);
    enemies.push_back(shroom);
    
    spdlog::info("Default enemies created");
}

void GameplayManager::setupProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float mapWidth = tilemap->getWidthInTiles() * tilemap->getTileWidth();
    float mapHeight = tilemap->getHeightInTiles() * tilemap->getTileHeight();
    glOrtho(0.0, mapWidth, mapHeight, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GameplayManager::handleLevelTransition() {
    if (levelTransitionCooldown > 0.0f) return;
    
    int tileW = tilemap->getTileWidth();
    int tileH = tilemap->getTileHeight();
    int playerTileX = static_cast<int>(player->getX() / tileW);
    int playerTileY = static_cast<int>(player->getY() / tileH);
    int gid = tilemap->getNormalizedTileIdAt(playerTileX, playerTileY);
    bool onGate = (gid == 120 || gid == 121 || gid == 122 || gid == 123);
    bool anyEnemyAlive = std::any_of(enemies.begin(), enemies.end(), [](Enemy* e){ return e && e->isAlive(); });
    
    if (onGate && !anyEnemyAlive) {
        spdlog::info("Gate passed on tileID {} at (x={} y={}). Resetting to center and respawning enemies on same map.", gid, playerTileX, playerTileY);
        
        // Clear projectiles
        playerProjectiles.clear();
        enemyProjectiles.clear();
        
        // Clear blood effects
        for (auto& bloodEffect : bloodEffects) {
            delete bloodEffect;
        }
        bloodEffects.clear();
        
        // Respawn enemies
        respawnEnemies();
        
        // Teleport player to map center
        teleportPlayerToCenter();
        
        // Regenerate player's HP to full on gate entry
        int healAmount = player->getMaxHealth() - player->getCurrentHealth();
        if (healAmount > 0) {
            player->heal(healAmount);
        }
        
        // Prevent immediate retriggering
        levelTransitionCooldown = 0.5f;
    }
}

void GameplayManager::updateGameLogic(float deltaTime, GLFWwindow* window) {
    // Decrease level transition cooldown
    if (levelTransitionCooldown > 0.0f) {
        levelTransitionCooldown -= deltaTime;
    }
    
    // Handle level transitions
    handleLevelTransition();
    
    // Process input
    bool anyEnemyAliveForMove = std::any_of(enemies.begin(), enemies.end(), [](Enemy* e){ return e && e->isAlive(); });
    bool gateOpenForMove = !anyEnemyAliveForMove;
    inputHandler->processInput(window, *player, deltaTime, *tilemap, playerProjectiles, gateOpenForMove);
}

void GameplayManager::updateEntities(float deltaTime) {
    // Update enemies
    for (auto& enemy : enemies) {
        if (enemy) {
            enemy->update(deltaTime, player->getX(), player->getY(), *tilemap, enemyProjectiles);
            enemy->updateAnimation(deltaTime);
        }
    }
    
    // Update projectiles
    for (auto& projectile : playerProjectiles) {
        projectile.update(deltaTime);
    }
    
    for (auto& projectile : enemyProjectiles) {
        projectile.update(deltaTime);
    }
    
    // Update blood effects
    for (auto& bloodEffect : bloodEffects) {
        if (bloodEffect) {
            bloodEffect->update(deltaTime);
        }
    }
}

void GameplayManager::handleCollisions() {
    // Handle player-enemy collisions
    collisionManager.handlePlayerEnemyCollisions(player, enemies);
    
    // Handle enemy-to-enemy collisions
    collisionManager.handleEnemyEnemyCollisions(enemies);
    
    // Handle projectile-wall collisions
    collisionManager.handleProjectileWallCollisions(playerProjectiles, enemyProjectiles, *tilemap);
    
    // Handle projectile-entity collisions
    collisionManager.handleProjectileCollisions(playerProjectiles, enemyProjectiles, player, enemies);
}

void GameplayManager::createBloodEffects() {
    for (auto& enemy : enemies) {
        if (enemy && enemy->shouldCreateBloodEffect()) {
            bloodEffects.push_back(new BloodEffect(enemy->getX(), enemy->getY() + 12, assetPath)); // Move blood 12px down
            enemy->markBloodEffectCreated();
            spdlog::info("Blood effect created at enemy death position ({}, {})", enemy->getX(), enemy->getY());
        }
    }
}

void GameplayManager::cleanupInactiveObjects() {
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
}

void GameplayManager::drawGameWorld() {
    tilemap->draw();
}

void GameplayManager::drawUI(int windowWidth, int windowHeight) {
    if (player) {
        UI::drawPlayerHealth(player->getCurrentHealth(), player->getMaxHealth(), windowWidth, windowHeight);
        UI::drawXPBar(player->getCurrentXP(), player->getMaxXP(), windowWidth, windowHeight);
        UI::drawLevelIndicator(player->getLevel(), windowWidth, windowHeight);
    }
}

void GameplayManager::drawEntities() {
    if (player) {
        player->draw();
    }
    
    for (auto& enemy : enemies) {
        if (enemy) {
            enemy->draw();
        }
    }
}

void GameplayManager::drawProjectiles() {
    for (auto& projectile : playerProjectiles) {
        projectile.draw();
    }
    
    for (auto& projectile : enemyProjectiles) {
        projectile.draw();
    }
}

void GameplayManager::drawBloodEffects() {
    for (auto& bloodEffect : bloodEffects) {
        if (bloodEffect) {
            bloodEffect->draw();
        }
    }
}

void GameplayManager::loadLevel(const std::string& levelPath) {
    spdlog::info("Loading map from JSON: {}", levelPath);
    if (!tilemap->loadFromJSON(levelPath)) {
        spdlog::error("Failed to load map from JSON.");
        return;
    }
    setupProjection();
    spdlog::info("Level loaded successfully");
}

void GameplayManager::respawnEnemies() {
    // Delete existing enemies
    for (auto& enemy : enemies) {
        if (enemy) {
            delete enemy;
        }
    }
    enemies.clear();
    
    // Create new enemies
    createDefaultEnemies();
    spdlog::info("Enemies respawned");
}

void GameplayManager::teleportPlayerToCenter() {
    if (!tilemap || !player) return;
    
    float mapWidth = tilemap->getWidthInTiles() * tilemap->getTileWidth();
    float mapHeight = tilemap->getHeightInTiles() * tilemap->getTileHeight();
    float centerX = mapWidth * 0.5f;
    float centerY = mapHeight * 0.5f;
    float dx = centerX - player->getX();
    float dy = centerY - player->getY();
    player->move(dx, dy);
    
    spdlog::info("Player teleported to center of map");
}
