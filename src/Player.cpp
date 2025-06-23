#include "Player.h"
#include <GLFW/glfw3.h>

Player::Player() : x(0.0f), y(0.0f) {}

void Player::move(float dx, float dy) {
    x += dx;
    y += dy;
}

void Player::draw() const {
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + 0.1f);    // Top vertex
    glVertex2f(x - 0.1f, y - 0.1f); // Bottom-left vertex
    glVertex2f(x + 0.1f, y - 0.1f); // Bottom-right vertex
    glEnd();
}