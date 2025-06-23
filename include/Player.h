#pragma once

class Player {
private:
    float x, y; // Position of the triangle
public:
    Player();
    void move(float dx, float dy);
    void draw() const;
};