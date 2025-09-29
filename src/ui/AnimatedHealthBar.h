#pragma once
#include <string>

class AnimatedHealthBar {
public:
    AnimatedHealthBar();
    ~AnimatedHealthBar();
    
    void initialize(const std::string& assetPath);
    void update(float deltaTime);
    void draw(int currentHealth, int maxHealth, int windowWidth, int windowHeight);
    void cleanup();

private:
    // Array of texture IDs for different health levels (100%, 80%, 60%, 40%, 20%, 0%)
    unsigned int healthTextures[6];
    int textureWidth, textureHeight;
    int frameWidth, frameHeight;
    
    // Health bar properties
    float barWidth;
    float barHeight;
    float barX, barY;
    
    bool initialized;
    
    void loadHealthTextures(const std::string& assetPath);
    void cleanupTextures();
    int getHealthLevelIndex(int currentHealth, int maxHealth);
};

