#include "CollisionManager.h"
#include "core/GameplayManager.h"
#include <spdlog/spdlog.h>
#include <algorithm>

CollisionManager::CollisionManager() {
    // Constructor - no initialization needed for now
}

void CollisionManager::handlePlayerEnemyCollisions(Player* player, const std::vector<Enemy*>& enemies) {
    if (!player) return;
    
    bool playerCollidingWithEnemy = false;
    
    for (auto& enemy : enemies) {
        if (!enemy->isAlive()) continue;
        
        // Quick distance check first to avoid expensive collision calculations
        float dx = player->getX() - enemy->getX();
        float dy = player->getY() - enemy->getY();
        float distanceSquared = dx * dx + dy * dy;
        
        if (distanceSquared < MAX_COLLISION_DISTANCE * MAX_COLLISION_DISTANCE) {
            // Check if player and enemy bounding boxes overlap
            bool collision = checkBoundingBoxCollision(
                player->getLeft(), player->getRight(), player->getTop(), player->getBottom(),
                enemy->getLeft(), enemy->getRight(), enemy->getTop(), enemy->getBottom()
            );
            
            if (collision) {
                playerCollidingWithEnemy = true;
                separatePlayerFromEnemy(player, enemy);
            }
        }
    }
    
    // Update player collision state
    player->setCollidingWithEnemy(playerCollidingWithEnemy);
}

void CollisionManager::handleEnemyEnemyCollisions(std::vector<Enemy*>& enemies) {
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (!enemies[i]->isAlive()) continue;
        
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            if (!enemies[j]->isAlive()) continue;
            
            // Quick distance check first
            float dx = enemies[i]->getX() - enemies[j]->getX();
            float dy = enemies[i]->getY() - enemies[j]->getY();
            float distanceSquared = dx * dx + dy * dy;
            
            if (distanceSquared < MAX_COLLISION_DISTANCE * MAX_COLLISION_DISTANCE) {
                // Check if enemy bounding boxes overlap
                bool collision = checkBoundingBoxCollision(
                    enemies[i]->getLeft(), enemies[i]->getRight(), enemies[i]->getTop(), enemies[i]->getBottom(),
                    enemies[j]->getLeft(), enemies[j]->getRight(), enemies[j]->getTop(), enemies[j]->getBottom()
                );
                
                if (collision) {
                    separateEnemies(enemies[i], enemies[j]);
                }
            }
        }
    }
}

void CollisionManager::handleProjectileWallCollisions(std::vector<Projectile>& playerProjectiles, 
                                                     std::vector<Projectile>& enemyProjectiles, 
                                                     const Tilemap& tilemap) {
    // Handle player projectile wall collisions
    for (auto& projectile : playerProjectiles) {
        if (projectile.checkWallCollision(tilemap)) {
            projectile.setActive(false);
            spdlog::info("Player projectile destroyed by wall collision");
        }
    }
    
    // Handle enemy projectile wall collisions
    for (auto& projectile : enemyProjectiles) {
        if (projectile.checkWallCollision(tilemap)) {
            projectile.setActive(false);
            spdlog::info("Enemy projectile destroyed by wall collision");
        }
    }
}

void CollisionManager::handleProjectileCollisions(std::vector<Projectile>& playerProjectiles,
                                                 std::vector<Projectile>& enemyProjectiles,
                                                 Player* player,
                                                 std::vector<Enemy*>& enemies,
                                                 class GameplayManager* gameplayManager) {
    // Player projectiles vs Enemy
    for (auto& projectile : playerProjectiles) {
        if (!projectile.isActive()) continue;
        
        for (auto& enemy : enemies) {
            if (!enemy->isAlive()) continue;
            
            if (projectile.checkCollision(enemy->getX(), enemy->getY(), PROJECTILE_COLLISION_RADIUS)) {
                projectile.setActive(false);
                enemy->takeDamage(PLAYER_PROJECTILE_DAMAGE, player);  // Deal damage and pass player for XP reward
                spdlog::info("Enemy hit by player projectile! Enemy HP: {}/{}", 
                            enemy->getCurrentHealth(), enemy->getMaxHealth());
                
                // Spawn damage number at enemy position
                if (gameplayManager) {
                    gameplayManager->spawnDamageNumber(enemy->getX(), enemy->getY() - 20, PLAYER_PROJECTILE_DAMAGE, false);
                }
                
                break; // Projectile can only hit one enemy
            }
        }
    }
    
    // Enemy projectiles vs Player
    for (auto& projectile : enemyProjectiles) {
        if (!projectile.isActive()) continue;
        
        if (projectile.checkCollision(player->getX(), player->getY(), PROJECTILE_COLLISION_RADIUS)) {
            projectile.setActive(false);
            player->takeDamage(ENEMY_PROJECTILE_DAMAGE);  // Deal damage
            spdlog::info("Player hit by enemy projectile! Player HP: {}/{}", 
                        player->getCurrentHealth(), player->getMaxHealth());
            
            // Spawn damage number at player position
            if (gameplayManager) {
                gameplayManager->spawnDamageNumber(player->getX(), player->getY() - 20, ENEMY_PROJECTILE_DAMAGE, true);
            }
        }
    }
}

bool CollisionManager::checkBoundingBoxCollision(float left1, float right1, float top1, float bottom1,
                                                float left2, float right2, float top2, float bottom2) {
    return !(right1 < left2 || left1 > right2 || bottom1 < top2 || top1 > bottom2);
}

void CollisionManager::separatePlayerFromEnemy(Player* player, Enemy* enemy) {
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
        // Separate horizontally (player moves left, enemy moves right)
        player->move(-separationAmount, 0);
        enemy->move(separationAmount, 0);
    } else if (minOverlap == overlapRight) {
        // Separate horizontally (player moves right, enemy moves left)
        player->move(separationAmount, 0);
        enemy->move(-separationAmount, 0);
    } else if (minOverlap == overlapTop) {
        // Separate vertically (player moves up, enemy moves down)
        player->move(0, -separationAmount);
        enemy->move(0, separationAmount);
    } else if (minOverlap == overlapBottom) {
        // Separate vertically (player moves down, enemy moves up)
        player->move(0, separationAmount);
        enemy->move(0, -separationAmount);
    }
}

void CollisionManager::separateEnemies(Enemy* enemy1, Enemy* enemy2) {
    // Calculate overlap amounts
    float overlapLeft = enemy1->getRight() - enemy2->getLeft();
    float overlapRight = enemy2->getRight() - enemy1->getLeft();
    float overlapTop = enemy1->getBottom() - enemy2->getTop();
    float overlapBottom = enemy2->getBottom() - enemy1->getTop();
    
    // Find the minimum overlap to determine separation direction
    float minOverlap = std::min({overlapLeft, overlapRight, overlapTop, overlapBottom});
    
    // Separate entities by moving them apart
    float separationAmount = minOverlap * 0.5f; // Half overlap to each entity
    
    if (minOverlap == overlapLeft) {
        // Separate horizontally (enemy1 moves left, enemy2 moves right)
        enemy1->move(-separationAmount, 0);
        enemy2->move(separationAmount, 0);
    } else if (minOverlap == overlapRight) {
        // Separate horizontally (enemy1 moves right, enemy2 moves left)
        enemy1->move(separationAmount, 0);
        enemy2->move(-separationAmount, 0);
    } else if (minOverlap == overlapTop) {
        // Separate vertically (enemy1 moves up, enemy2 moves down)
        enemy1->move(0, -separationAmount);
        enemy2->move(0, separationAmount);
    } else if (minOverlap == overlapBottom) {
        // Separate vertically (enemy1 moves down, enemy2 moves up)
        enemy1->move(0, separationAmount);
        enemy2->move(0, -separationAmount);
    }
}

float CollisionManager::calculateOverlap(float min1, float max1, float min2, float max2) {
    float overlap = std::min(max1, max2) - std::max(min1, min2);
    return (overlap > 0) ? overlap : 0;
}
