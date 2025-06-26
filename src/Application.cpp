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

    // Set up viewport and orthographic projection
    int windowWidth = 1920;
    int windowHeight = 1080;
    glfwSetWindowSize(window, windowWidth, windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    Player player;
    stbi_set_flip_vertically_on_load(true);
    player.loadTexture("../assets/graphic/Vampire_Walk.png", 64, 64, 4);
    player.loadIdleTexture("../assets/graphic/Vampire_Idle.png", 64, 64, 2);
    stbi_set_flip_vertically_on_load(false);
    
    // Create enemy
    Enemy enemy(20 * 16.0f, 15 * 16.0f, EnemyType::Skeleton); // Position at tile (20, 15)
    stbi_set_flip_vertically_on_load(true);
    enemy.loadTexture("../assets/graphic/skeleton_enemy.png", 64, 64, 4);
    stbi_set_flip_vertically_on_load(false);
    
    // Projectile vectors
    std::vector<Projectile> playerProjectiles;
    std::vector<Projectile> enemyProjectiles;
    
    InputHandler inputHandler;
    Tilemap tilemap;
    if (!tilemap.loadTilesetTexture("../assets/maps/catacombs.png", 16, 16)) {
        spdlog::error("Failed to load tileset texture");
        return -1;
    }
    if (!tilemap.loadFromJSON("../assets/maps/test.json")) {
        spdlog::error("Failed to load map from JSON.");
        return -1;
    }

    // Set up projection to match tilemap size
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float mapWidth = tilemap.getWidthInTiles() * tilemap.getTileWidth();
    float mapHeight = tilemap.getHeightInTiles() * tilemap.getTileHeight();
    glOrtho(0.0, mapWidth, mapHeight, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT);

        inputHandler.processInput(window, player, deltaTime, tilemap, playerProjectiles);
    
        tilemap.draw();
        player.draw();
        
        // Update and draw enemy
        enemy.update(deltaTime, player.getX(), player.getY(), tilemap);
        enemy.updateAnimation(deltaTime);
        enemy.draw();
        
        // Enemy shooting
        enemy.shootProjectile(player.getX(), player.getY(), enemyProjectiles);
        
        // Update and draw projectiles
        for (auto& projectile : playerProjectiles) {
            projectile.update(deltaTime);
            projectile.draw();
        }
        
        for (auto& projectile : enemyProjectiles) {
            projectile.update(deltaTime);
            projectile.draw();
        }
        
        // Collision detection
        // Player projectiles vs Enemy
        for (auto& projectile : playerProjectiles) {
            if (projectile.isActive() && enemy.isAlive()) {
                if (projectile.checkCollision(enemy.getX(), enemy.getY(), 8.0f)) {
                    projectile.setActive(false);
                    enemy.takeDamage(25);  // Deal 25 damage
                    spdlog::info("Enemy hit by player projectile! Enemy HP: {}/{}", 
                                enemy.getCurrentHealth(), enemy.getMaxHealth());
                }
            }
        }
        
        // Enemy projectiles vs Player
        for (auto& projectile : enemyProjectiles) {
            if (projectile.isActive()) {
                if (projectile.checkCollision(player.getX(), player.getY(), 8.0f)) {
                    projectile.setActive(false);
                    player.takeDamage(15);  // Deal 15 damage
                    spdlog::info("Player hit by enemy projectile! Player HP: {}/{}", 
                                player.getCurrentHealth(), player.getMaxHealth());
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
        UI::drawPlayerHealth(player.getCurrentHealth(), player.getMaxHealth(), windowWidth, windowHeight);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    spdlog::info("Shutting down Ortos II application");
    glfwTerminate();
    return 0;
}