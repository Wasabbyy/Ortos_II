#pragma once
#include <vector>
#include <string>

class BloodEffect {
public:
    BloodEffect(float x, float y, const std::string& assetPath = "");
    ~BloodEffect();
    
    void update(float deltaTime);
    void draw() const;
    bool isActive() const { return active; }
    bool isFinished() const { return finished; }
    float getX() const { return x; }
    float getY() const { return y; }

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
    void loadBloodTextures(const std::string& assetPath);
    void cleanupTextures();
}; 