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
    std::cout << "=== Starting JSON Load ===" << std::endl;
    std::cout << "Loading file: " << jsonPath << std::endl;

    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        std::cerr << "ERROR: Failed to open JSON file!" << std::endl;
        return false;
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << "ERROR parsing JSON: " << e.what() << std::endl;
        return false;
    }

    // Print basic map info
    std::cout << "\n=== Map Info ===" << std::endl;
    std::cout << "Map size: " << j["width"] << "x" << j["height"] << std::endl;
    std::cout << "Tile size: " << j["tilewidth"] << "x" << j["tileheight"] << std::endl;
    std::cout << "Number of layers: " << j["layers"].size() << std::endl;

    // Load tileset
    std::string resolvedPath = "../assets/maps/catacombs.tsx";
    std::cout << "\nLoading tileset from: " << resolvedPath << std::endl;
    
    if (!loadTilesetFromTSX(resolvedPath)) {
        std::cerr << "ERROR: Failed to load tileset!" << std::endl;
        return false;
    }

    // Store map dimensions
    width = j["width"];
    height = j["height"];
    tileWidth = j["tilewidth"];
    tileHeight = j["tileheight"];

    // Clear existing layers
    layers.clear();

    // Process each layer
    std::cout << "\n=== Processing Layers ===" << std::endl;
    for (const auto& layer : j["layers"]) {
        std::cout << "\nLayer: " << layer["name"] << std::endl;
        std::cout << "Type: " << layer["type"] << std::endl;
        std::cout << "Visible: " << layer["visible"] << std::endl;
        std::cout << "Opacity: " << layer["opacity"] << std::endl;

        if (!layer.contains("data")) {
            std::cerr << "WARNING: Layer missing data, skipping" << std::endl;
            continue;
        }

        const auto& data = layer["data"];
        std::cout << "Tile count: " << data.size() << std::endl;

        // Print first 10 tiles for debugging
        std::cout << "First 10 tiles: ";
        for (int i = 0; i < 10 && i < data.size(); ++i) {
            std::cout << data[i] << " ";
        }
        std::cout << std::endl;

        // Add new layer
        layers.emplace_back();
        auto& currentLayer = layers.back();
        currentLayer.resize(height, std::vector<int>(width, 0));

        // Load tile data
        int emptyTiles = 0;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int index = y * width + x;
                if (index >= data.size()) {
                    std::cerr << "ERROR: Index out of bounds at (" << x << "," << y << ")" << std::endl;
                    continue;
                }
                currentLayer[y][x] = data[index];
                if (data[index] == 0) emptyTiles++;
            }
        }

        std::cout << "Loaded " << (width * height - emptyTiles) << " non-empty tiles (" 
                  << emptyTiles << " empty tiles)" << std::endl;
    }

    std::cout << "\n=== Load Complete ===" << std::endl;
    std::cout << "Successfully loaded " << layers.size() << " layers" << std::endl;
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
    if (textureID == 0) {
        std::cerr << "ERROR: No texture loaded!" << std::endl;
        return;
    }

    if (layers.empty()) {
        std::cerr << "ERROR: No layers to draw!" << std::endl;
        return;
    }

    std::cout << "\n=== Drawing Tilemap ===" << std::endl;
    std::cout << "Drawing " << layers.size() << " layers" << std::endl;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    float scaleX = static_cast<float>(1920) / (tileWidth * width);
    float scaleY = static_cast<float>(1080) / (tileHeight * height);

    for (size_t i = 0; i < layers.size(); ++i) {
        std::cout << "Drawing layer " << i << " with " 
                  << width * height << " tiles" << std::endl;

        const auto& layer = layers[i];
        int tilesDrawn = 0;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int tileID = layer[y][x];
                if (tileID == 0) continue; 

                tilesDrawn++;
                int tilesPerRow = textureWidth / tileWidth;
                int tileIndex = tileID - 1;

                int tileX = tileIndex % tilesPerRow;
                int tileY = tileIndex / tilesPerRow;

                const float padding = 0.5f; 

                float u1 = (tileX * tileWidth + padding) / textureWidth;
                float v1 = (tileY * tileHeight + padding) / textureHeight;
                float u2 = ((tileX + 1) * tileWidth - padding) / textureWidth;
                float v2 = ((tileY + 1) * tileHeight - padding) / textureHeight;


                float worldX = x * tileWidth * scaleX + offsetX;
                float worldY = y * tileHeight * scaleY + offsetY;
                float scaledTileWidth = tileWidth * scaleX;
                float scaledTileHeight = tileHeight * scaleY;

                std::swap(v1, v2);

                glBegin(GL_QUADS);
                    glTexCoord2f(u1, v2); glVertex2f(worldX, worldY);
                    glTexCoord2f(u2, v2); glVertex2f(worldX + scaledTileWidth, worldY);
                    glTexCoord2f(u2, v1); glVertex2f(worldX + scaledTileWidth, worldY + scaledTileHeight);
                    glTexCoord2f(u1, v1); glVertex2f(worldX, worldY + scaledTileHeight);
                glEnd();
            }
        }

        std::cout << "Drew " << tilesDrawn << " non-empty tiles in layer " << i << std::endl;
    }

    glDisable(GL_TEXTURE_2D);
    std::cout << "Finished drawing tilemap" << std::endl;
}
int Tilemap::getTileAt(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) {
        return -1;
    }
    return tiles[y][x];
}