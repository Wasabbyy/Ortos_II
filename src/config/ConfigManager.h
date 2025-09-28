#pragma once

#include <string>
#include <map>
#include <fstream>
#include <spdlog/spdlog.h>

class ConfigManager {
private:
    std::string configFilePath;
    std::map<std::string, std::string> configData;
    bool configLoaded;

public:
    ConfigManager();
    ~ConfigManager();

    // Initialize with config file path
    void initialize(const std::string& configPath);

    // Load config from file
    bool loadConfig();

    // Save config to file
    bool saveConfig();

    // Get string value with default
    std::string getString(const std::string& key, const std::string& defaultValue = "");

    // Get float value with default
    float getFloat(const std::string& key, float defaultValue = 0.0f);

    // Get int value with default
    int getInt(const std::string& key, int defaultValue = 0);

    // Get bool value with default
    bool getBool(const std::string& key, bool defaultValue = false);

    // Set string value
    void setString(const std::string& key, const std::string& value);

    // Set float value
    void setFloat(const std::string& key, float value);

    // Set int value
    void setInt(const std::string& key, int value);

    // Set bool value
    void setBool(const std::string& key, bool value);

    // Check if config is loaded
    bool isLoaded() const { return configLoaded; }

    // Get config file path
    std::string getConfigFilePath() const { return configFilePath; }
};
