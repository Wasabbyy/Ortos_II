#include <GLFW/glfw3.h>
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
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
    if (!UI::init("../assets/fonts/Arial.ttf")) {
        spdlog::error("Failed to initialize UI system");
        glfwTerminate();
        return -1;
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
    
    // Input debouncing
    bool keyUpPressed = false;
    bool keyDownPressed = false;
    bool keyEnterPressed = false;
    
    // Mouse input for death screen
    double mouseX = 0.0, mouseY = 0.0;
    bool mouseLeftPressed = false;
    bool respawnButtonHovered = false;

    // Game objects (will be initialized when starting game)
    Player* player = nullptr;
    Enemy* enemy = nullptr;
    std::vector<Projectile> playerProjectiles;
    std::vector<Projectile> enemyProjectiles;
    InputHandler* inputHandler = nullptr;
    Tilemap* tilemap = nullptr;

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT);

        if (currentState == GameState::MENU) {
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
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyEnterPressed) {
                if (selectedMenuOption == 0) {
                    currentState = GameState::PLAYING;
                } else if (selectedMenuOption == 1) {
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
            }

            // Game logic
            inputHandler->processInput(window, *player, deltaTime, *tilemap, playerProjectiles);
        
            tilemap->draw();
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
                        spdlog::info("Player hit by enemy projectile! Player HP: {}/{}", 
                                    player->getCurrentHealth(), player->getMaxHealth());
                    }
                }
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
                currentState = GameState::MENU;
                spdlog::info("Returning to menu");
            }
            
            // Check if player has died
            if (!player->isAlive()) {
                currentState = GameState::DEATH;
                spdlog::info("Player has died, showing death screen");
            }
        }
        else if (currentState == GameState::DEATH) {
            // Handle mouse input for death screen
            glfwGetCursorPos(window, &mouseX, &mouseY);
            float buttonWidth = 200.0f;
            float buttonHeight = 60.0f;
            float buttonX = windowWidth / 2.0f - buttonWidth / 2.0f;
            float respawnButtonY = windowHeight * 0.5f;
            float exitButtonY = windowHeight * 0.35f;

            // Track which button is selected (0 = respawn, 1 = exit)
            static int selectedDeathButton = 0;
            bool respawnButtonHovered = UI::isMouseOverButton(mouseX, mouseY, buttonX, respawnButtonY, buttonWidth, buttonHeight);
            bool exitButtonHovered = UI::isMouseOverButton(mouseX, mouseY, buttonX, exitButtonY, buttonWidth, buttonHeight);

            // Keyboard navigation
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !keyUpPressed) {
                selectedDeathButton = (selectedDeathButton - 1 + 2) % 2;
                keyUpPressed = true;
            } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
                keyUpPressed = false;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !keyDownPressed) {
                selectedDeathButton = (selectedDeathButton + 1) % 2;
                keyDownPressed = true;
            } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
                keyDownPressed = false;
            }

            // Mouse hover overrides selection
            if (respawnButtonHovered) selectedDeathButton = 0;
            if (exitButtonHovered) selectedDeathButton = 1;

            // Mouse click
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mouseLeftPressed) {
                if (respawnButtonHovered) {
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
                    currentState = GameState::PLAYING;
                } else if (exitButtonHovered) {
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
                    currentState = GameState::PLAYING;
                } else if (selectedDeathButton == 1) {
                    spdlog::info("Enter pressed on Exit, exiting game");
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

    // Cleanup UI system
    UI::cleanup();

    spdlog::info("Shutting down Ortos II application");
    glfwTerminate();
    return 0;
}