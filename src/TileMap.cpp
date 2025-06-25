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
    
    return true;
}

bool Tilemap::loadFromJSON(const std::string& jsonPath) {
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

    // Load tileset
    std::string resolvedPath = "../assets/maps/catacombs.tsx";
    if (!loadTilesetFromTSX(resolvedPath)) {
        std::cerr << "ERROR: Failed to load tileset!" << std::endl;
        return false;
    }

    // Store map dimensions
    width = j["width"];
    height = j["height"];
    tileWidth = j["tilewidth"];
    tileHeight = j["tileheight"];

    // Clear existing layers and collision data
    layers.clear();
    collisionLayer.clear();

    // Process each layer
    for (const auto& layer : j["layers"]) {
        std::string layerName = layer["name"];
        std::string layerType = layer["type"];
        bool visible = layer["visible"];
        int layerId = layer["id"];

        if (!layer.contains("data")) {
            continue;
        }

        const auto& data = layer["data"];

        // Handle collision layer
        if (layerId == 3) {
            collisionLayer.resize(height, std::vector<int>(width, 0));
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int index = y * width + x;
                    if (index < data.size()) {
                        collisionLayer[y][x] = data[index];
                    }
                }
            }
            continue;
        }

        // Handle visible tile layers
        if (layerType == "tilelayer") {
            layers.emplace_back();
            auto& currentLayer = layers.back();
            currentLayer.resize(height, std::vector<int>(width, 0));

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int index = y * width + x;
                    if (index < data.size()) {
                        currentLayer[y][x] = data[index];
                    }
                }
            }
        }
    }

    return true;
}

void Tilemap::draw(float offsetX, float offsetY) const {
    if (textureID == 0 || layers.empty()) {
        return;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    for (const auto& layer : layers) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int tileID = layer[y][x];
                if (tileID == 0) continue;

                int tilesPerRow = textureWidth / tileWidth;
                int tileIndex = tileID - 1;

                int tileX = tileIndex % tilesPerRow;
                int tileY = tileIndex / tilesPerRow;

                const float padding = 0.5f;
                float u1 = (tileX * tileWidth + padding) / textureWidth;
                float v1 = (tileY * tileHeight + padding) / textureHeight;
                float u2 = ((tileX + 1) * tileWidth - padding) / textureWidth;
                float v2 = ((tileY + 1) * tileHeight - padding) / textureHeight;

                float worldX = x * tileWidth + offsetX;
                float worldY = y * tileHeight + offsetY;

                std::swap(v1, v2);

                glBegin(GL_QUADS);
                    glTexCoord2f(u1, v2); glVertex2f(worldX, worldY);
                    glTexCoord2f(u2, v2); glVertex2f(worldX + tileWidth, worldY);
                    glTexCoord2f(u2, v1); glVertex2f(worldX + tileWidth, worldY + tileHeight);
                    glTexCoord2f(u1, v1); glVertex2f(worldX, worldY + tileHeight);
                glEnd();
            }
        }
    }

    glDisable(GL_TEXTURE_2D);
}

bool Tilemap::isTileSolid(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) return true;
    if (collisionLayer.empty()) return false;
    return collisionLayer[y][x] != 0;  // Assuming 0 means walkable
}

int Tilemap::getWidthInTiles() const { return width; }
int Tilemap::getHeightInTiles() const { return height; }
int Tilemap::getTileWidth() const { return tileWidth; }
int Tilemap::getTileHeight() const { return tileHeight; }
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
