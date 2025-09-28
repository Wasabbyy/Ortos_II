#pragma once
#include <vector>
#include <string>

class GateEffect {
public:
    GateEffect(float x, float y, const std::string& assetPath = "");
    ~GateEffect();
    
    void update(float deltaTime);
    void draw() const;
    bool isActive() const { return active; }
    bool isFinished() const { return finished; }
    float getX() const { return x; }
    float getY() const { return y; }
    void stopLooping() { looping = false; }

private:
    float x, y;
    bool active;
    bool finished;
    
    // Animation properties
    float animationTimer;
    float frameDuration;
    int currentFrame;
    int totalFrames;
    bool looping;
    
    // Texture for gate effects spritesheet
    unsigned int gateTextureID;
    int textureWidth, textureHeight;
    int frameWidth, frameHeight;
    int framesPerRow;
    int totalRows;
    
    // Load gate effect texture
    void loadGateTexture(const std::string& assetPath);
    void cleanupTexture();
};
