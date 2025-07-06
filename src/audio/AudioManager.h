#pragma once
#include <AL/al.h>
#include <AL/alc.h>
#include <string>
#include <vector>
#include <map>
#include <memory>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    // Initialize OpenAL
    bool init();
    void cleanup();

    // Load and play sound effects
    bool loadSound(const std::string& name, const std::string& filePath);
    void playSound(const std::string& name, float volume = 1.0f);
    void playSound3D(const std::string& name, float x, float y, float z, float volume = 1.0f);
    
    // Background music
    bool loadMusic(const std::string& name, const std::string& filePath);
    void playMusic(const std::string& name, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    ALuint getMusicSource() const { return musicSource; }
    ALCcontext* getContext() const { return context; }
    
    // Audio settings
    void setMasterVolume(float volume);
    void setSoundVolume(float volume);
    void setMusicVolume(float volume);
    
    // 3D audio settings
    void setListenerPosition(float x, float y, float z);
    void setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                               float upX, float upY, float upZ);

private:
    ALCdevice* device;
    ALCcontext* context;
    
    // Sound effects
    std::map<std::string, ALuint> soundBuffers;
    std::vector<ALuint> soundSources;
    
    // Background music
    std::map<std::string, ALuint> musicBuffers;
    ALuint musicSource;
    std::string currentMusic;
    
    // Volume settings
    float masterVolume;
    float soundVolume;
    float musicVolume;
    
    // Helper functions
    bool loadWAVFile(const std::string& filePath, ALuint& buffer);
    ALuint getAvailableSource();
    void releaseSource(ALuint source);
}; 