#include "AudioManager.h"
#include <spdlog/spdlog.h>
#include <fstream>
#include <iostream>

AudioManager::AudioManager()
    : device(nullptr), context(nullptr), musicSource(0),
      masterVolume(1.0f), soundVolume(1.0f), musicVolume(1.0f) {
}

AudioManager::~AudioManager() {
    cleanup();
}

bool AudioManager::init() {
    spdlog::info("AudioManager::init() - Starting initialization");
    
    // Open default device
    device = alcOpenDevice(nullptr);
    if (!device) {
        spdlog::error("Failed to open OpenAL device");
        return false;
    }
    spdlog::info("OpenAL device opened successfully");

    // Create context
    context = alcCreateContext(device, nullptr);
    if (!context) {
        spdlog::error("Failed to create OpenAL context");
        return false;
    }
    spdlog::info("OpenAL context created successfully");

    // Make context current
    if (!alcMakeContextCurrent(context)) {
        spdlog::error("Failed to make OpenAL context current");
        return false;
    }
    spdlog::info("OpenAL context made current successfully");

    // Initialize ALUT (if available)
    // alutInit(nullptr, nullptr);

    // Set listener properties
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    ALfloat orientation[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
    alListenerfv(AL_ORIENTATION, orientation);

    // Create sound sources (pool of sources for sound effects)
    soundSources.resize(16); // 16 simultaneous sound effects
    alGenSources(soundSources.size(), soundSources.data());
    
    // Create music source
    alGenSources(1, &musicSource);
    
    spdlog::info("AudioManager::init() - Initialization completed successfully");
    return true;
}

void AudioManager::cleanup() {
    if (context) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        context = nullptr;
    }
    
    if (device) {
        alcCloseDevice(device);
        device = nullptr;
    }
    
    // Clean up sources
    if (!soundSources.empty()) {
        alDeleteSources(soundSources.size(), soundSources.data());
        soundSources.clear();
    }
    
    if (musicSource) {
        alDeleteSources(1, &musicSource);
        musicSource = 0;
    }
    
    // Clean up buffers
    for (auto& pair : soundBuffers) {
        alDeleteBuffers(1, &pair.second);
    }
    soundBuffers.clear();
    
    // Clean up music buffers
    for (auto& pair : musicBuffers) {
        alDeleteBuffers(1, &pair.second);
    }
    musicBuffers.clear();
}

bool AudioManager::loadSound(const std::string& name, const std::string& filePath) {
    ALuint buffer;
    if (!loadWAVFile(filePath, buffer)) {
        spdlog::error("Failed to load sound: {}", filePath);
        return false;
    }
    
    soundBuffers[name] = buffer;
    spdlog::info("Loaded sound: {} from {}", name, filePath);
    return true;
}

void AudioManager::playSound(const std::string& name, float volume) {
    auto it = soundBuffers.find(name);
    if (it == soundBuffers.end()) {
        spdlog::warn("Sound not found: {}", name);
        return;
    }
    
    ALuint source = getAvailableSource();
    if (source == AL_NONE) {
        spdlog::warn("No available sound sources");
        return;
    }
    
    alSourcei(source, AL_BUFFER, it->second);
    alSourcef(source, AL_GAIN, volume * soundVolume * masterVolume);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcePlay(source);
}

void AudioManager::playSound3D(const std::string& name, float x, float y, float z, float volume) {
    auto it = soundBuffers.find(name);
    if (it == soundBuffers.end()) {
        spdlog::warn("Sound not found: {}", name);
        return;
    }
    
    ALuint source = getAvailableSource();
    if (source == AL_NONE) {
        spdlog::warn("No available sound sources");
        return;
    }
    
    alSourcei(source, AL_BUFFER, it->second);
    alSourcef(source, AL_GAIN, volume * soundVolume * masterVolume);
    alSource3f(source, AL_POSITION, x, y, z);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcePlay(source);
}

bool AudioManager::loadMusic(const std::string& name, const std::string& filePath) {
    ALuint buffer;
    if (!loadWAVFile(filePath, buffer)) {
        spdlog::error("Failed to load music file: {}", filePath);
        return false;
    }
    
    musicBuffers[name] = buffer;
    spdlog::info("Loaded music: {} from {}", name, filePath);
    return true;
}

void AudioManager::playMusic(const std::string& name, bool loop) {
    auto it = musicBuffers.find(name);
    if (it == musicBuffers.end()) {
        spdlog::error("Music not loaded: {}", name);
        return;
    }
    
    // Stop current music if playing
    if (musicSource != 0) {
        alSourceStop(musicSource);
        alSourcei(musicSource, AL_BUFFER, 0);
    }
    
    // Create new source if needed
    if (musicSource == 0) {
        alGenSources(1, &musicSource);
        spdlog::debug("Created music source: {}", musicSource);
    }
    
    // Set source properties
    alSourcei(musicSource, AL_BUFFER, it->second);
    alSourcei(musicSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcef(musicSource, AL_GAIN, musicVolume * masterVolume);
    
    // Play the music
    alSourcePlay(musicSource);
    
    // Check for errors
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        spdlog::error("OpenAL error playing music: {}", error);
    } else {
        spdlog::info("Started playing music: {} (source: {})", name, musicSource);
    }
}

void AudioManager::stopMusic() {
    alSourceStop(musicSource);
}

void AudioManager::pauseMusic() {
    alSourcePause(musicSource);
}

void AudioManager::resumeMusic() {
    alSourcePlay(musicSource);
}

void AudioManager::setMasterVolume(float volume) {
    masterVolume = std::max(0.0f, std::min(1.0f, volume));
    // Update current music volume
    alSourcef(musicSource, AL_GAIN, musicVolume * masterVolume);
}

void AudioManager::setSoundVolume(float volume) {
    soundVolume = std::max(0.0f, std::min(1.0f, volume));
}

void AudioManager::setMusicVolume(float volume) {
    musicVolume = std::max(0.0f, std::min(1.0f, volume));
    alSourcef(musicSource, AL_GAIN, musicVolume * masterVolume);
}

void AudioManager::setListenerPosition(float x, float y, float z) {
    alListener3f(AL_POSITION, x, y, z);
}

void AudioManager::setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                         float upX, float upY, float upZ) {
    float orientation[] = {forwardX, forwardY, forwardZ, upX, upY, upZ};
    alListenerfv(AL_ORIENTATION, orientation);
}

bool AudioManager::loadWAVFile(const std::string& filePath, ALuint& buffer) {
    spdlog::info("loadWAVFile() - Attempting to load: {}", filePath);
    
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        spdlog::error("Failed to open audio file: {}", filePath);
        return false;
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    spdlog::info("File opened successfully, size: {} bytes", static_cast<long>(fileSize));
    
    // Read WAV header
    char header[44];
    file.read(header, 44);
    
    // Check if it's a valid WAV file
    if (header[0] != 'R' || header[1] != 'I' || header[2] != 'F' || header[3] != 'F' ||
        header[8] != 'W' || header[9] != 'A' || header[10] != 'V' || header[11] != 'E') {
        spdlog::error("Invalid WAV file format: {}", filePath);
        return false;
    }
    spdlog::info("WAV header validated successfully");
    
    // Extract format information (little-endian)
    int channels = static_cast<unsigned char>(header[22]) | (static_cast<unsigned char>(header[23]) << 8);
    int sampleRate = static_cast<unsigned char>(header[24]) | (static_cast<unsigned char>(header[25]) << 8) | 
                     (static_cast<unsigned char>(header[26]) << 16) | (static_cast<unsigned char>(header[27]) << 24);
    int bitsPerSample = static_cast<unsigned char>(header[34]) | (static_cast<unsigned char>(header[35]) << 8);
    
    spdlog::info("WAV format: {} channels, {} Hz, {} bits", channels, sampleRate, bitsPerSample);
    
    // Find data chunk
    char chunkHeader[8];
    int dataSize = 0;
    bool dataChunkFound = false;
    
    // Start from the beginning of the file after the main header
    file.seekg(12, std::ios::beg);
    
    while (file.read(chunkHeader, 8) && !dataChunkFound) {
        int chunkSize = static_cast<unsigned char>(chunkHeader[4]) | (static_cast<unsigned char>(chunkHeader[5]) << 8) | 
                       (static_cast<unsigned char>(chunkHeader[6]) << 16) | (static_cast<unsigned char>(chunkHeader[7]) << 24);
        
        if (chunkHeader[0] == 'd' && chunkHeader[1] == 'a' && chunkHeader[2] == 't' && chunkHeader[3] == 'a') {
            dataSize = chunkSize;
            dataChunkFound = true;
            spdlog::info("Found data chunk, size: {} bytes", dataSize);
            break;
        } else {
            spdlog::debug("Skipping chunk: {}{}{}{}, size: {}", 
                         chunkHeader[0], chunkHeader[1], chunkHeader[2], chunkHeader[3], chunkSize);
            file.seekg(chunkSize, std::ios::cur);
        }
    }
    
    if (!dataChunkFound) {
        spdlog::error("No data chunk found in WAV file: {}", filePath);
        return false;
    }
    
    // Read audio data
    std::vector<char> audioData(dataSize);
    file.read(audioData.data(), dataSize);
    spdlog::info("Read {} bytes of audio data", dataSize);
    
    // Determine OpenAL format
    ALenum format;
    if (channels == 1) {
        format = (bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    } else {
        format = (bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    }
    
    spdlog::info("Using OpenAL format: {}", format);
    
    // Create OpenAL buffer
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, audioData.data(), dataSize, sampleRate);
    
    // Check for errors
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        spdlog::error("OpenAL error creating buffer: {}", error);
        alDeleteBuffers(1, &buffer);
        return false;
    }
    
    spdlog::info("Successfully loaded WAV file: {} ({} Hz, {} channels, {} bits, {} bytes)", 
                 filePath, sampleRate, channels, bitsPerSample, dataSize);
    return true;
}

ALuint AudioManager::getAvailableSource() {
    for (ALuint source : soundSources) {
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED) {
            return source;
        }
    }
    return AL_NONE;
}

void AudioManager::releaseSource(ALuint source) {
    alSourceStop(source);
    alSourcei(source, AL_BUFFER, 0);
} 