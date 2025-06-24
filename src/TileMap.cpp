// Tilemap.cpp
#include "TileMap.h"
#include <fstream>
#include <sstream>
#include <stb_image.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include "tinyxml2.h"
using json = nlohmann::json;

Tilemap::Tilemap() : textureID(0), tileWidth(0), tileHeight(0), 
                    width(0), height(0) {}

Tilemap::~Tilemap() {
    if (textureID) {
        glDeleteTextures(1, &textureID);
    }
}

bool Tilemap::loadTilesetTexture(const std::string& imagePath, int tileW, int tileH) {
    this->tileWidth = tileW;
    this->tileHeight = tileH;

    int width, height, channels;
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load tileset texture: " << imagePath << std::endl;
        return false;
    }
    std::cout << "Texture loaded: " << imagePath << " (" << width << "x" << height << ")" << std::endl;
    textureWidth = width;
    textureHeight = height;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum format = GL_RGBA;
    if (channels == 3) format = GL_RGB;
    else if (channels == 1) format = GL_RED;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    std::cout << "Loaded tileset: " << imagePath << std::endl;
    std::cout << "Size: " << width << "x" << height << " (" << channels << " channels)" << std::endl;
    std::cout << "Tile size: " << tileW << "x" << tileH << std::endl;
    std::cout << "Texture ID: " << textureID << std::endl;
    
    return true;
}

bool Tilemap::loadFromJSON(const std::string& jsonPath) {
    std::cout << "Attempting to load JSON file: " << jsonPath << std::endl;

    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonPath << std::endl;
        return false;
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON file: " << e.what() << std::endl;
        return false;
    }

    if (!j.contains("tilesets") || !j["tilesets"].is_array() || j["tilesets"].empty()) {
        std::cerr << "Missing 'tilesets' in JSON." << std::endl;
        return false;
    }

    std::string resolvedPath = "/Users/filipstupar/Documents/OrtosII/assets/maps/catacombs.tsx";
    std::cout << "Hardcoded TSX path: " << resolvedPath << std::endl;
    
    if (!loadTilesetFromTSX(resolvedPath)) {
        std::cerr << "Failed to load tileset from: " << resolvedPath << std::endl;
        return false;
    }

    // Get map size and tile size
    width = j["width"];
    height = j["height"];
    tileWidth = j["tilewidth"];
    tileHeight = j["tileheight"];
    std::cout << "Map dimensions: " << width << "x" << height << std::endl;
    std::cout << "Tile dimensions: " << tileWidth << "x" << tileHeight << std::endl;
    std::cout << "Tilemap dimensions: " << width << "x" << height << std::endl;

    

    // Get the first (base) layer data
    if (!j.contains("layers") || !j["layers"].is_array() || j["layers"].empty()) {
        std::cerr << "Invalid or missing 'layers' in JSON file." << std::endl;
        return false;
    }
    
    const auto& layer = j["layers"][0];
    if (!layer.contains("data") || !layer["data"].is_array()) {
        std::cerr << "Missing tile data in layer." << std::endl;
        return false;
    }
    
    const auto& data = layer["data"];
    if (data.size() != width * height) {
        std::cerr << "Tile data size mismatch. Expected: " << (width * height) 
                  << ", Found: " << data.size() << std::endl;
        return false;
    }
    
    tiles.resize(height, std::vector<int>(width, 0));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            if (index >= data.size()) {
                std::cerr << "Error: Out-of-bounds access in tile data (index = " << index << ")." << std::endl;
                return false;
            }
            tiles[y][x] = data[index];
        }
    }

    std::cout << "Successfully loaded tilemap from JSON: " << jsonPath << std::endl;
    std::cout << "Tile data:" << std::endl;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            std::cout << tiles[y][x] << " ";
        }
        std::cout << std::endl;
    }
    return true;
}

bool Tilemap::loadTilesetFromTSX(const std::string& tsxPath) {
    std::cout << "Attempting to load TSX file: " << tsxPath << std::endl;
    

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(tsxPath.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Failed to load TSX file: " << tsxPath << std::endl;
        std::cerr << "Error: " << doc.ErrorStr() << std::endl;
        return false;
    }

    auto* tileset = doc.FirstChildElement("tileset");
    if (!tileset) {
        std::cerr << "No <tileset> element in TSX" << std::endl;
        return false;
    }

    tileWidth = tileset->IntAttribute("tilewidth");
    tileHeight = tileset->IntAttribute("tileheight");
    std::cout << "Tileset dimensions: " << tileWidth << "x" << tileHeight << std::endl;

    auto* image = tileset->FirstChildElement("image");
    if (!image) {
        std::cerr << "No <image> in TSX" << std::endl;
        return false;
    }

    std::string imagePath = image->Attribute("source");
    std::cout << "Found tileset image source: " << imagePath << std::endl;

    // You could optionally normalize path here (TSX may use relative path)
    return loadTilesetTexture(imagePath, tileWidth, tileHeight);
}


void Tilemap::draw(float offsetX, float offsetY) const {
    if (textureID == 0 || tiles.empty()) {
        std::cerr << "Error: Texture ID is 0 or tiles are empty." << std::endl;
        return;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Calculate scale to stretch the tilemap across the screen
    float scaleX = static_cast<float>(1920) / (tileWidth * width);
    float scaleY = static_cast<float>(1080) / (tileHeight * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int tileID = tiles[y][x];
            if (tileID == 0) continue; // Skip empty tiles

            int tilesPerRow = textureWidth / tileWidth;
            int tileIndex = tileID - 1;

            int tileX = tileIndex % tilesPerRow;
            int tileY = tileIndex / tilesPerRow;

            float u1 = static_cast<float>(tileX * tileWidth) / textureWidth;
            float v1 = static_cast<float>(tileY * tileHeight) / textureHeight;
            float u2 = static_cast<float>((tileX + 1) * tileWidth) / textureWidth;
            float v2 = static_cast<float>((tileY + 1) * tileHeight) / textureHeight;

            float worldX = x * tileWidth * scaleX + offsetX;
            float worldY = y * tileHeight * scaleY + offsetY;
            float scaledTileWidth = tileWidth * scaleX;
            float scaledTileHeight = tileHeight * scaleY;

            glBegin(GL_QUADS);
                glTexCoord2f(u1, v2); glVertex2f(worldX, worldY);
                glTexCoord2f(u2, v2); glVertex2f(worldX + scaledTileWidth, worldY);
                glTexCoord2f(u2, v1); glVertex2f(worldX + scaledTileWidth, worldY + scaledTileHeight);
                glTexCoord2f(u1, v1); glVertex2f(worldX, worldY + scaledTileHeight);
            glEnd();
        }
    }

    glDisable(GL_TEXTURE_2D);
}

int Tilemap::getTileAt(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) {
        return -1; // Out of bounds
    }
    return tiles[y][x];
}