#pragma once
#include <string>

class DamageNumber {
public:
    DamageNumber(float x, float y, int damage, bool isPlayerDamage);
    ~DamageNumber();
    
    void update(float deltaTime);
    void draw() const;
    bool isActive() const { return active; }
    bool isFinished() const { return finished; }
    float getX() const { return x; }
    float getY() const { return y; }

private:
    float x, y;
    int damage;
    bool isPlayerDamage;  // true = player took damage (red), false = enemy took damage (yellow)
    bool active;
    bool finished;
    
    // Animation properties
    float lifetime;
    float maxLifetime;
    float floatSpeed;
    float alpha;  // Fade out effect
    
    // Helper methods to draw text in world space
    void drawTextInWorldSpace(const std::string& text, float worldX, float worldY, 
                               float scale, float r, float g, float b, float a) const;
    void drawDigitShape(int digit, float x, float y, float pixelSize) const;
}; 
