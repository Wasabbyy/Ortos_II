// Tilemap.h
#pragma once
#include <vector>
#include <string>
#include <unordered_set>
#include <GLFW/glfw3.h>

class Tilemap {
public:
    Tilemap();
    ~Tilemap();
    
    bool loadTilesetTexture(const std::string& imagePath, int tileWidth, int tileHeight);
    void draw(float offsetX = 0, float offsetY = 0) const;
    bool loadFromJSON(const std::string& jsonPath);
    bool loadTilesetFromTSX(const std::string& tsxPath);
    bool isTileSolid(int x, int y) const;
    int getNormalizedTileIdAt(int x, int y) const;
    int getTileWidth() const;
    int getTileHeight() const;
    int getWidthInTiles() const;
    int getHeightInTiles() const;

private:
    unsigned int textureID;
    int textureWidth, textureHeight;
    int tileWidth, tileHeight;
    int width, height;

    std::vector<std::vector<std::vector<int>>> layers; // Drawable layers
    std::vector<std::vector<int>> collisionLayer;      // Collision layer
};