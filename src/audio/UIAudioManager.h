#pragma once
#include <AL/al.h>
#include <AL/alc.h>
#include <string>
#include <vector>
#include <map>

class UIAudioManager {
public:
    UIAudioManager();
    ~UIAudioManager();

    // Initialize with existing OpenAL context (shared with main AudioManager)
    bool init(ALCcontext* existingContext = nullptr);
    void cleanup();

    // Load and play UI sound effects
    bool loadUISound(const std::string& name, const std::string& filePath);
    void playUISound(const std::string& name, float volume = 1.0f);
    
    // UI-specific sound functions
    void playButtonHoverSound();
    void playButtonClickSound();
    
    // Audio settings
    void setUIVolume(float volume);
    float getUIVolume() const { return uiVolume; }

private:
    ALCdevice* device;
    ALCcontext* context;
    bool ownsContext; // Track if we own the context or just use an existing one
    
    // UI sound effects
    std::map<std::string, ALuint> uiSoundBuffers;
    std::vector<ALuint> uiSoundSources;
    
    // Volume settings
    float uiVolume;
    
    
    // Helper functions
    bool loadWAVFile(const std::string& filePath, ALuint& buffer);
    ALuint getAvailableUISource();
    void releaseUISource(ALuint source);
}; 