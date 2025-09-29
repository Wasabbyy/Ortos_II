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
    unsigned int handsTextureID;
    int textureWidth, textureHeight;
    int frameWidth, frameHeight;
    int framesPerRow;
    int totalRows;
    int totalFrames;
    
    // Animation properties
    float animationTimer;
    float frameDuration;
    int currentFrame;
    
    // Health bar properties
    float barWidth;
    float barHeight;
    float barX, barY;
    
    bool initialized;
    
    void loadHandsTexture(const std::string& assetPath);
    void cleanupTexture();
};

