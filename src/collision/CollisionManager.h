#pragma once

#include <vector>
#include "../player/Player.h"
#include "../enemy/Enemy.h"
#include "../projectile/Projectile.h"
#include "../map/TileMap.h"

class CollisionManager {
public:
    CollisionManager();
    
    // Player-Enemy collision detection
    void handlePlayerEnemyCollisions(Player* player, const std::vector<Enemy*>& enemies);
    
    // Enemy-to-Enemy collision detection
    void handleEnemyEnemyCollisions(std::vector<Enemy*>& enemies);
    
    // Projectile-wall collision detection
    void handleProjectileWallCollisions(std::vector<Projectile>& playerProjectiles, 
                                       std::vector<Projectile>& enemyProjectiles, 
                                       const Tilemap& tilemap);
    
    // Projectile-entity collision detection
    void handleProjectileCollisions(std::vector<Projectile>& playerProjectiles,
                                   std::vector<Projectile>& enemyProjectiles,
                                   Player* player,
                                   std::vector<Enemy*>& enemies);

private:
    // Helper functions for collision detection
    bool checkBoundingBoxCollision(float left1, float right1, float top1, float bottom1,
                                  float left2, float right2, float top2, float bottom2);
    
    void separatePlayerFromEnemy(Player* player, Enemy* enemy);
    void separateEnemies(Enemy* enemy1, Enemy* enemy2);
    
    float calculateOverlap(float min1, float max1, float min2, float max2);
    
    // Constants
    static constexpr float MAX_COLLISION_DISTANCE = 64.0f;
    static constexpr float PROJECTILE_COLLISION_RADIUS = 8.0f;
    static constexpr int PLAYER_PROJECTILE_DAMAGE = 20;  // Adjusted to match 6 health levels (5 hits = 100% to 0%)
    static constexpr int ENEMY_PROJECTILE_DAMAGE = 20;   // Adjusted to match 6 health levels (5 hits = 100% to 0%)
};
