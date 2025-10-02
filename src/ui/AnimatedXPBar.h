#pragma once
#include <string>

class AnimatedXPBar {
public:
    AnimatedXPBar();
    ~AnimatedXPBar();
    
    void initialize(const std::string& assetPath);
    void update(float deltaTime);
    void draw(int currentXP, int maxXP, int windowWidth, int windowHeight);
    void drawWithState(int xpState, int windowWidth, int windowHeight);
    void cleanup();

private:
    // Array of texture IDs for different XP states (xpbar_01 to xpbar_05)
    unsigned int xpTextures[5];
    int textureWidth, textureHeight;
    int frameWidth, frameHeight;
    
    // XP bar properties
    float barWidth;
    float barHeight;
    float barX, barY;
    
    bool initialized;
    
    void loadXPTextures(const std::string& assetPath);
    void cleanupTextures();
    int getXPStateIndex(int currentXP, int maxXP);
};
