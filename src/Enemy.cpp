#include "Enemy.h"
#include "Projectile.h"
#include "TileMap.h"
#include "UI.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <spdlog/spdlog.h>
#include <cmath>
#include <random>

Enemy::Enemy(float x, float y, EnemyType type)
    : x(x), y(y), type(type),
      textureID(0),
      frameWidth(0), frameHeight(0),
      textureWidth(0), textureHeight(0),
      totalFrames(1),
      animationSpeed(0.8f), elapsedTime(0.0f),
      currentFrame(0), alive(true), state(EnemyState::Idle),
      startX(x), startY(y),
      gen(rd()), randomDir(-1.0f, 1.0f),
      bloodEffectCreated(false) {
    
    // Set different properties based on enemy type
    switch (type) {
        case EnemyType::FlyingEye:
            moveSpeed = 80.0f;  // Faster than skeleton
            patrolRadius = 120.0f;
            chaseRadius = 180.0f;
            shootInterval = 1.5f;  // Shoot more frequently
            shootRange = 250.0f;   // Longer range
            maxHealth = 150;       // More health to see hit animation
            currentHealth = 150;
            animationSpeed = 0.2f; // Very fast animation
            break;
        case EnemyType::Shroom:
            moveSpeed = 40.0f;     // Slower than flying eye
            patrolRadius = 80.0f;
            chaseRadius = 140.0f;
            shootInterval = 2.5f;  // Shoot less frequently
            shootRange = 180.0f;   // Medium range
            maxHealth = 200;       // More health to see hit animation
            currentHealth = 200;
            animationSpeed = 0.25f; // Very fast animation speed
            break;
        case EnemyType::Skeleton:
        case EnemyType::Zombie:
        case EnemyType::Ghost:
        default:
            // Default properties (same as before)
            moveSpeed = 50.0f;
            patrolRadius = 100.0f;
            chaseRadius = 150.0f;
            shootInterval = 2.0f;
            shootRange = 200.0f;
            maxHealth = 100;
            currentHealth = 100;
            animationSpeed = 0.3f; // Very fast default animation
            break;
    }
    
    spdlog::debug("Enemy created at position ({}, {}) with type {}", x, y, static_cast<int>(type));
}

Enemy::~Enemy() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
    if (hitTextureID != 0) {
        glDeleteTextures(1, &hitTextureID);
    }
}

void Enemy::draw() const {
    if (!alive) return;
    
    // Determine which texture to use
    bool useHitTexture = isHitAnimationActive && hitTextureID != 0;
    unsigned int currentTextureID = useHitTexture ? hitTextureID : textureID;
    
    // If no texture is available, don't draw
    if (currentTextureID == 0) return;

    glEnable(GL_TEXTURE_2D);
    
    // Choose which texture properties to use
    int currentFrameWidth = useHitTexture ? hitFrameWidth : frameWidth;
    int currentFrameHeight = useHitTexture ? hitFrameHeight : frameHeight;
    int currentTextureWidth = useHitTexture ? hitTextureWidth : textureWidth;
    int currentTextureHeight = useHitTexture ? hitTextureHeight : textureHeight;
    int currentTotalFrames = useHitTexture ? hitTotalFrames : totalFrames;
    int currentFrameIndex = useHitTexture ? hitCurrentFrame : currentFrame;
    
    // Fallback to normal texture properties if hit texture properties are invalid
    if (useHitTexture && (currentFrameWidth == 0 || currentFrameHeight == 0)) {
        useHitTexture = false;
        currentTextureID = textureID;
        currentFrameWidth = frameWidth;
        currentFrameHeight = frameHeight;
        currentTextureWidth = textureWidth;
        currentTextureHeight = textureHeight;
        currentTotalFrames = totalFrames;
        currentFrameIndex = currentFrame;
    }
    
    glBindTexture(GL_TEXTURE_2D, currentTextureID);

    // Calculate texture coordinates for current frame
    int framesPerRow = currentTextureWidth / currentFrameWidth;
    int row, col;
    
    if (useHitTexture) {
        // For hit animation, use row 0 (first row) since hit textures are 600x150 with 4 frames horizontally
        row = 0;
        col = currentFrameIndex % framesPerRow;
    } else {
        // For normal animation, use the existing logic
        row = currentFrameIndex / framesPerRow;
        col = currentFrameIndex % framesPerRow;
    }

    float u1 = static_cast<float>(col * currentFrameWidth) / currentTextureWidth;
    float v1 = static_cast<float>(row * currentFrameHeight) / currentTextureHeight;
    float u2 = static_cast<float>((col + 1) * currentFrameWidth) / currentTextureWidth;
    float v2 = static_cast<float>((row + 1) * currentFrameHeight) / currentTextureHeight;

    // Flip texture coordinates if facing left
    if (!facingRight) {
        float temp = u1;
        u1 = u2;
        u2 = temp;
    }

    // Draw enemy centered on tile
    float drawX = x - currentFrameWidth / 2.0f;
    float drawY = y - currentFrameHeight / 2.0f;

    // Special effect for flying eye
    if (type == EnemyType::FlyingEye) {
        // Add a subtle glow effect
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(0.8f, 0.4f, 1.0f, 0.3f); // Purple glow
        glBegin(GL_QUADS);
        glTexCoord2f(u1, v2); glVertex2f(drawX - 2, drawY - 2);
        glTexCoord2f(u2, v2); glVertex2f(drawX + currentFrameWidth + 2, drawY - 2);
        glTexCoord2f(u2, v1); glVertex2f(drawX + currentFrameWidth + 2, drawY + currentFrameHeight + 2);
        glTexCoord2f(u1, v1); glVertex2f(drawX - 2, drawY + currentFrameHeight + 2);
        glEnd();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Reset color
    }
    
    // Special effect for shroom
    if (type == EnemyType::Shroom) {
        // Add a green glow effect
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(0.2f, 0.8f, 0.3f, 0.4f); // Green glow
        glBegin(GL_QUADS);
        glTexCoord2f(u1, v2); glVertex2f(drawX - 3, drawY - 3);
        glTexCoord2f(u2, v2); glVertex2f(drawX + currentFrameWidth + 3, drawY - 3);
        glTexCoord2f(u2, v1); glVertex2f(drawX + currentFrameWidth + 3, drawY + currentFrameHeight + 3);
        glTexCoord2f(u1, v1); glVertex2f(drawX - 3, drawY + currentFrameHeight + 3);
        glEnd();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Reset color
    }

    glBegin(GL_QUADS);
    glTexCoord2f(u1, v2); glVertex2f(drawX, drawY);
    glTexCoord2f(u2, v2); glVertex2f(drawX + currentFrameWidth, drawY);
    glTexCoord2f(u2, v1); glVertex2f(drawX + currentFrameWidth, drawY + currentFrameHeight);
    glTexCoord2f(u1, v1); glVertex2f(drawX, drawY + currentFrameHeight);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // Draw health bar right above enemy hitbox
    UI::drawEnemyHealthBar(x, getTop() - 3.0f, currentHealth, maxHealth);

    // Draw collision rectangle (bounding box) in different colors for different enemy types
    glLineWidth(2.0f);
    glDisable(GL_BLEND);
    if (type == EnemyType::FlyingEye) {
        glColor3f(1.0f, 0.0f, 1.0f); // Magenta for flying eye
    } else if (type == EnemyType::Shroom) {
        glColor3f(0.0f, 1.0f, 0.0f); // Green for shroom
    } else {
        glColor3f(0.0f, 0.0f, 1.0f); // Blue for other enemies
    }
    glBegin(GL_LINE_LOOP);
    glVertex2f(getLeft(), getTop());
    glVertex2f(getRight(), getTop());
    glVertex2f(getRight(), getBottom());
    glVertex2f(getLeft(), getBottom());
    glEnd();
    glEnable(GL_BLEND);
    glLineWidth(1.0f);
    glColor3f(1.0f, 1.0f, 1.0f); // Reset color to white
}

void Enemy::loadTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames) {
    this->frameWidth = frameWidth;
    this->frameHeight = frameHeight;
    this->totalFrames = totalFrames;

    int width, height, channels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        spdlog::error("Failed to load enemy texture: {}", filePath);
        return;
    }

    spdlog::info("Loaded enemy texture: {} ({}x{})", filePath, width, height);

    textureWidth = width;
    textureHeight = height;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Use NEAREST filtering for pixel-perfect graphics
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum format = GL_RGB;
    if (channels == 4) format = GL_RGBA;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 1) format = GL_RED;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    spdlog::debug("Enemy texture loaded successfully with ID: {}", textureID);
}

void Enemy::loadHitTexture(const std::string& filePath, int frameWidth, int frameHeight, int totalFrames) {
    this->hitFrameWidth = frameWidth;
    this->hitFrameHeight = frameHeight;
    this->hitTotalFrames = totalFrames;

    int width, height, channels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        spdlog::error("Failed to load enemy hit texture: {}", filePath);
        return;
    }

    spdlog::info("Loaded enemy hit texture: {} ({}x{})", filePath, width, height);

    hitTextureWidth = width;
    hitTextureHeight = height;

    glGenTextures(1, &hitTextureID);
    glBindTexture(GL_TEXTURE_2D, hitTextureID);

    // Use NEAREST filtering for pixel-perfect graphics
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum format = GL_RGB;
    if (channels == 4) format = GL_RGBA;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 1) format = GL_RED;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    spdlog::debug("Enemy hit texture loaded successfully with ID: {}", hitTextureID);
}

void Enemy::updateAnimation(float deltaTime) {
    if (!alive) return;
    
    // Update hit animation if active
    if (isHitAnimationActive) {
        hitAnimationTimer += deltaTime;
        
        // Update hit animation frames
        hitElapsedTime += deltaTime;
        if (hitElapsedTime >= hitAnimationSpeed) {
            hitElapsedTime -= hitAnimationSpeed;
            hitCurrentFrame = (hitCurrentFrame + 1) % hitTotalFrames;
            spdlog::debug("Hit animation frame: {}/{} for enemy type {}", hitCurrentFrame, hitTotalFrames, static_cast<int>(type));
        }
        
        // Check if hit animation should end
        if (hitAnimationTimer >= hitAnimationDuration) {
            isHitAnimationActive = false;
            hitAnimationTimer = 0.0f;
            hitCurrentFrame = 0;
            hitElapsedTime = 0.0f;
            spdlog::debug("Hit animation finished for enemy type {}", static_cast<int>(type));
        }
        return; // Don't update normal animation while hit animation is active
    }
    
    // Update normal animation
    elapsedTime += deltaTime;
    if (elapsedTime >= animationSpeed) {
        elapsedTime -= animationSpeed;
        currentFrame = (currentFrame + 1) % totalFrames;
    }
}

void Enemy::move(float dx, float dy) {
    if (!alive) return;
    
    float oldX = x;
    float oldY = y;
    
    x += dx;
    y += dy;
    
    // Update facing direction based on horizontal movement
    if (dx != 0) {
        lastMoveX = dx;
        facingRight = (dx > 0);
    }
    
    spdlog::debug("Enemy moved from ({}, {}) to ({}, {})", oldX, oldY, x, y);
}

void Enemy::shootProjectile(float targetX, float targetY, std::vector<Projectile>& projectiles) {
    if (!alive || shootCooldown > 0) return;
    
    float dx = targetX - x;
    float dy = targetY - y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance <= shootRange) {
        projectiles.emplace_back(x, y, dx, dy, ProjectileType::EnemyBullet);
        shootCooldown = shootInterval;
        spdlog::debug("Enemy shot projectile at player");
    }
}

void Enemy::update(float deltaTime, float playerX, float playerY, const Tilemap& tilemap) {
    if (!alive) return;
    
    // Update shooting cooldown
    if (shootCooldown > 0) {
        shootCooldown -= deltaTime;
    }
    
    // Calculate distance to player
    float dx = playerX - x;
    float dy = playerY - y;
    float distanceToPlayer = std::sqrt(dx * dx + dy * dy);
    
    // State machine for enemy behavior
    if (distanceToPlayer <= chaseRadius) {
        state = EnemyState::Chasing;
        spdlog::debug("Enemy chasing player at distance: {}", distanceToPlayer);
    } else if (distanceToPlayer <= patrolRadius) {
        state = EnemyState::Patrolling;
        spdlog::debug("Enemy patrolling near player at distance: {}", distanceToPlayer);
    } else {
        state = EnemyState::Idle;
    }
    
    float moveX = 0.0f, moveY = 0.0f;
    
    switch (state) {
        case EnemyState::Chasing: {
            // Move towards player
            if (distanceToPlayer > 0) {
                moveX = (dx / distanceToPlayer) * moveSpeed * deltaTime;
                moveY = (dy / distanceToPlayer) * moveSpeed * deltaTime;
                
                // Flying eye has more erratic movement
                if (type == EnemyType::FlyingEye) {
                    // Add some random movement to make it more unpredictable
                    float randomOffsetX = randomDir(gen) * 0.3f;
                    float randomOffsetY = randomDir(gen) * 0.3f;
                    moveX += randomOffsetX * moveSpeed * deltaTime;
                    moveY += randomOffsetY * moveSpeed * deltaTime;
                }
            }
            break;
        }
        case EnemyState::Patrolling: {
            // Simple patrol behavior - move back and forth
            patrolTimer += deltaTime;
            if (patrolTimer >= patrolDuration) {
                patrolTimer = 0.0f;
                currentPatrolDirection *= -1.0f;  // Change direction
            }
            
            // Patrol horizontally
            moveX = currentPatrolDirection * moveSpeed * 0.5f * deltaTime;
            
            // Flying eye patrols in a more complex pattern
            if (type == EnemyType::FlyingEye) {
                // Add vertical movement to patrol pattern
                moveY = std::sin(patrolTimer * 2.0f) * moveSpeed * 0.3f * deltaTime;
            }
            break;
        }
        case EnemyState::Idle:
        default: {
            // Random movement when out of range
            randomMoveTimer += deltaTime;
            if (randomMoveTimer >= randomMoveDuration) {
                randomMoveTimer = 0.0f;
                randomMoveX = randomDir(gen);
                randomMoveY = randomDir(gen);
                
                // Normalize random direction
                float length = std::sqrt(randomMoveX * randomMoveX + randomMoveY * randomMoveY);
                if (length > 0) {
                    randomMoveX /= length;
                    randomMoveY /= length;
                }
                
                spdlog::debug("Enemy new random direction: ({}, {})", randomMoveX, randomMoveY);
            }
            
            moveX = randomMoveX * moveSpeed * 0.3f * deltaTime;
            moveY = randomMoveY * moveSpeed * 0.3f * deltaTime;
            
            // Flying eye has more active idle movement
            if (type == EnemyType::FlyingEye) {
                moveX *= 1.5f;  // Move faster when idle
                moveY *= 1.5f;
            }
            break;
        }
    }
    
    // Collision detection (similar to player)
    if (moveX != 0.0f || moveY != 0.0f) {
        float left = getLeft();
        float right = getRight();
        float top = getTop();
        float bottom = getBottom();
        
        // Calculate new rectangle after movement
        float newLeft = left + moveX;
        float newRight = right + moveX;
        float newTop = top + moveY;
        float newBottom = bottom + moveY;
        
        // Check collision with tiles
        auto isSolid = [&](float px, float py) {
            int tileX = static_cast<int>(px / tilemap.getTileWidth());
            int tileY = static_cast<int>(py / tilemap.getTileHeight());
            return tilemap.isTileSolid(tileX, tileY);
        };
        
        bool canMoveX = true;
        if (moveX != 0) {
            float testX = moveX > 0 ? newRight : newLeft;
            if (isSolid(testX, top) || isSolid(testX, bottom - 1)) {
                canMoveX = false;
            }
        }
        
        bool canMoveY = true;
        if (moveY != 0) {
            float testY = moveY > 0 ? newBottom : newTop;
            if (isSolid(left, testY) || isSolid(right - 1, testY)) {
                canMoveY = false;
            }
        }
        
        // Apply movement
        if (canMoveX && canMoveY) {
            move(moveX, moveY);
        } else if (canMoveX) {
            move(moveX, 0);
        } else if (canMoveY) {
            move(0, moveY);
        } else {
            // Hit a wall, change direction
            if (state == EnemyState::Patrolling) {
                currentPatrolDirection *= -1.0f;
                patrolTimer = 0.0f;
            } else if (state == EnemyState::Idle) {
                // Change random direction when hitting wall
                randomMoveTimer = randomMoveDuration;  // Force new direction
            }
        }
    }
}

void Enemy::takeDamage(int damage) {
    currentHealth = std::max(0, currentHealth - damage);
    spdlog::info("Enemy took {} damage. Health: {}/{}", damage, currentHealth, maxHealth);
    
    // Trigger hit animation for FlyingEye and Shroom enemies
    if ((type == EnemyType::FlyingEye || type == EnemyType::Shroom) && hitTextureID != 0) {
        isHitAnimationActive = true;
        hitAnimationTimer = 0.0f;
        hitCurrentFrame = 0;
        hitElapsedTime = 0.0f;
        spdlog::debug("Hit animation triggered for enemy type {} at position ({}, {})", static_cast<int>(type), x, y);
    } else if ((type == EnemyType::FlyingEye || type == EnemyType::Shroom) && hitTextureID == 0) {
        spdlog::warn("Hit animation requested but hit texture not loaded for enemy type {}", static_cast<int>(type));
    }
    
    if (currentHealth <= 0) {
        alive = false;
        spdlog::warn("Enemy has been defeated!");
    }
}

void Enemy::heal(int amount) {
    currentHealth = std::min(maxHealth, currentHealth + amount);
    spdlog::info("Enemy healed {} HP. Health: {}/{}", amount, currentHealth, maxHealth);
} 