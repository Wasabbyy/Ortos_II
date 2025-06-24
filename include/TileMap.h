// Tilemap.h
#pragma once
#include <vector>
#include <string>
#include <GLFW/glfw3.h>

class Tilemap {
public:
    Tilemap();
    ~Tilemap();
    
    bool loadTilesetTexture(const std::string& imagePath, int tileWidth, int tileHeight);
    void draw(float offsetX = 0, float offsetY = 0) const;
    bool loadFromJSON(const std::string& jsonPath);
    bool loadTilesetFromTSX(const std::string& tsxPath); // Removed Tilemap:: here
    
    // Helper functions
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getTileAt(int x, int y) const;
    
private:
    std::vector<std::vector<int>> tiles;
    unsigned int textureID;
    int textureWidth, textureHeight;
    int tileWidth, tileHeight;
    int width, height;
};