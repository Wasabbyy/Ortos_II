#pragma once
#include <vector>
#include <string>

class BloodEffect {
public:
    BloodEffect(float x, float y);
    ~BloodEffect();

    void update(float deltaTime);
    void draw() const;
    bool isActive() const { return active; }
    bool isFinished() const { return finished; }

private:
    float x, y;
    bool active;
    bool finished;
    
    // Animation properties
    float animationTimer;
    float frameDuration;
    int currentFrame;
    int totalFrames;
    
    // Textures for blood frames
    std::vector<unsigned int> bloodTextures;
    int textureWidth, textureHeight;
    
    // Load blood textures
    void loadBloodTextures();
    void cleanupTextures();
}; 