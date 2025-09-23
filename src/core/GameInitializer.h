#pragma once

#include <GLFW/glfw3.h>
#include <AL/al.h>
#include <string>
#include <memory>

// Forward declarations
class AudioManager;
class UIAudioManager;

class GameInitializer {
public:
    GameInitializer();
    ~GameInitializer();

    // Main initialization method
    bool initialize();
    
    // Individual initialization methods
    bool initializeGraphics(GLFWwindow*& window);
    bool initializeAudio();
    bool initializeUI();
    bool initializeLogger();
    void shutdown();

    // Getters for initialized objects
    GLFWwindow* getWindow() const { return window; }
    AudioManager* getAudioManager() const { return audioManager.get(); }
    UIAudioManager* getUIAudioManager() const { return uiAudioManager.get(); }
    
    // Utility methods
    std::string getAssetPath(const std::string& relativePath = "");

private:
    // Window and graphics
    GLFWwindow* window;
    int windowWidth;
    int windowHeight;
    
    // Audio managers
    std::unique_ptr<AudioManager> audioManager;
    std::unique_ptr<UIAudioManager> uiAudioManager;
    
    // Initialization state
    bool graphicsInitialized;
    bool audioInitialized;
    bool uiInitialized;
    bool loggerInitialized;
    
    // Private helper methods
    bool setupOpenGL();
    bool setupProjection();
    void setupInputCallbacks();
};
