#include "audio/UIAudioManager.h"
#include <spdlog/spdlog.h>
#include <fstream>
#include <iostream>
#include <algorithm>

UIAudioManager::UIAudioManager()
    : device(nullptr), context(nullptr), ownsContext(false), uiVolume(0.7f) {
}

UIAudioManager::~UIAudioManager() {
    cleanup();
}

bool UIAudioManager::init(ALCcontext* existingContext) {
    spdlog::info("UIAudioManager::init() - Starting initialization");
    
    if (existingContext) {
        // Use existing context
        context = existingContext;
        ownsContext = false;
        spdlog::info("UIAudioManager using existing OpenAL context");
    } else {
        // Create new context (fallback, but not recommended)
        spdlog::warn("UIAudioManager creating new OpenAL context - this may cause conflicts!");
        
        // Open default device
        device = alcOpenDevice(nullptr);
        if (!device) {
            spdlog::error("Failed to open OpenAL device for UI audio");
            return false;
        }
        
        // Create context
        context = alcCreateContext(device, nullptr);
        if (!context) {
            spdlog::error("Failed to create OpenAL context for UI audio");
            return false;
        }
        
        // Make context current
        if (!alcMakeContextCurrent(context)) {
            spdlog::error("Failed to make OpenAL context current for UI audio");
            return false;
        }
        
        ownsContext = true;
        spdlog::info("UIAudioManager created new OpenAL context");
    }
    
    // Set listener properties (only if we own the context)
    if (ownsContext) {
        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        ALfloat orientation[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
        alListenerfv(AL_ORIENTATION, orientation);
    }

    // Create UI sound sources (separate pool for UI sounds)
    uiSoundSources.resize(8); // 8 simultaneous UI sound effects
    alGenSources(uiSoundSources.size(), uiSoundSources.data());
    
    // Check for errors in source creation
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        spdlog::error("OpenAL error creating UI sources: {}", error);
        return false;
    }
    
    // Initialize all sources to a known state
    for (ALuint source : uiSoundSources) {
        alSourcei(source, AL_BUFFER, 0);
        alSourcef(source, AL_GAIN, 1.0f);
        alSourcei(source, AL_LOOPING, AL_FALSE);
        alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
        alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        // Pre-configure for UI sounds - disable distance attenuation
        alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSourcef(source, AL_REFERENCE_DISTANCE, 1.0f);
        alSourcef(source, AL_MAX_DISTANCE, 1.0f);
    }
    
    spdlog::info("UIAudioManager::init() - Initialization completed successfully");
    return true;
}

void UIAudioManager::cleanup() {
    // Clean up UI sources
    if (!uiSoundSources.empty()) {
        alDeleteSources(uiSoundSources.size(), uiSoundSources.data());
        uiSoundSources.clear();
    }
    
    // Clean up UI buffers
    for (auto& pair : uiSoundBuffers) {
        alDeleteBuffers(1, &pair.second);
    }
    uiSoundBuffers.clear();
    
    // Only clean up context if we own it
    if (ownsContext) {
        if (context) {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(context);
            context = nullptr;
        }
        
        if (device) {
            alcCloseDevice(device);
            device = nullptr;
        }
    }
    
    ownsContext = false;
}

bool UIAudioManager::loadUISound(const std::string& name, const std::string& filePath) {
    ALuint buffer;
    if (!loadWAVFile(filePath, buffer)) {
        spdlog::error("Failed to load UI sound: {}", filePath);
        return false;
    }
    
    uiSoundBuffers[name] = buffer;
    spdlog::info("Loaded UI sound: {} from {}", name, filePath);
    return true;
}

void UIAudioManager::playUISound(const std::string& name, float volume) {
    auto it = uiSoundBuffers.find(name);
    if (it == uiSoundBuffers.end()) {
        spdlog::warn("UI sound not found: {}", name);
        return;
    }
    
    ALuint source = getAvailableUISource();
    if (source == AL_NONE) {
        spdlog::warn("No available UI sound sources");
        return;
    }
    
    // Set buffer and play immediately - minimal calls for lowest latency
    alSourcei(source, AL_BUFFER, it->second);
    alSourcef(source, AL_GAIN, volume * uiVolume);
    alSourcePlay(source);
    
    // Check for errors after playing
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        spdlog::error("OpenAL error playing UI sound: {}", error);
    }
}

void UIAudioManager::playButtonHoverSound() {
    // For now, we'll use the button sound for hover as well
    // You can add a separate hover sound file later
    playUISound("button", 0.5f); // Increased volume for hover
}

void UIAudioManager::playButtonClickSound() {
    playUISound("button", 0.9f); // Increased volume for click
}

void UIAudioManager::setUIVolume(float volume) {
    uiVolume = std::max(0.0f, std::min(1.0f, volume));
    spdlog::info("UI volume set to: {}", uiVolume);
}

bool UIAudioManager::loadWAVFile(const std::string& filePath, ALuint& buffer) {
    spdlog::info("UIAudioManager::loadWAVFile() - Attempting to load: {}", filePath);
    
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        spdlog::error("Failed to open UI audio file: {}", filePath);
        return false;
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    spdlog::info("UI audio file opened successfully, size: {} bytes", static_cast<long>(fileSize));
    
    // Read WAV header
    char header[44];
    file.read(header, 44);
    
    // Check if it's a valid WAV file
    if (header[0] != 'R' || header[1] != 'I' || header[2] != 'F' || header[3] != 'F' ||
        header[8] != 'W' || header[9] != 'A' || header[10] != 'V' || header[11] != 'E') {
        spdlog::error("Invalid WAV file format for UI audio: {}", filePath);
        return false;
    }
    spdlog::info("WAV header validated successfully for UI audio");
    
    // Extract format information (little-endian)
    int channels = static_cast<unsigned char>(header[22]) | (static_cast<unsigned char>(header[23]) << 8);
    int sampleRate = static_cast<unsigned char>(header[24]) | (static_cast<unsigned char>(header[25]) << 8) | 
                     (static_cast<unsigned char>(header[26]) << 16) | (static_cast<unsigned char>(header[27]) << 24);
    int bitsPerSample = static_cast<unsigned char>(header[34]) | (static_cast<unsigned char>(header[35]) << 8);
    
    spdlog::info("UI WAV format: {} channels, {} Hz, {} bits", channels, sampleRate, bitsPerSample);
    
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
            spdlog::info("Found data chunk for UI audio, size: {} bytes", dataSize);
            break;
        } else {
            spdlog::debug("Skipping chunk: {}{}{}{}, size: {}", 
                         chunkHeader[0], chunkHeader[1], chunkHeader[2], chunkHeader[3], chunkSize);
            file.seekg(chunkSize, std::ios::cur);
        }
    }
    
    if (!dataChunkFound) {
        spdlog::error("No data chunk found in UI WAV file: {}", filePath);
        return false;
    }
    
    // Read audio data
    std::vector<char> audioData(dataSize);
    file.read(audioData.data(), dataSize);
    spdlog::info("Read {} bytes of UI audio data", dataSize);
    
    // Determine OpenAL format
    ALenum format;
    if (channels == 1) {
        format = (bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    } else {
        format = (bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    }
    
    spdlog::info("Using OpenAL format for UI audio: {}", format);
    
    // Create OpenAL buffer
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, audioData.data(), dataSize, sampleRate);
    
    // Check for errors
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        spdlog::error("OpenAL error creating UI buffer: {}", error);
        alDeleteBuffers(1, &buffer);
        return false;
    }
    
    spdlog::info("Successfully loaded UI WAV file: {} ({} Hz, {} channels, {} bits, {} bytes)", 
                 filePath, sampleRate, channels, bitsPerSample, dataSize);
    return true;
}

ALuint UIAudioManager::getAvailableUISource() {
    for (ALuint source : uiSoundSources) {
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        
        // Check if source is stopped or initial state
        if (state == AL_STOPPED || state == AL_INITIAL) {
            // Minimal reset - just clear the buffer and play
            alSourcei(source, AL_BUFFER, 0);
            return source;
        }
    }
    
    // If no sources are available, force stop the first one and reuse it
    if (!uiSoundSources.empty()) {
        ALuint source = uiSoundSources[0];
        alSourceStop(source);
        alSourcei(source, AL_BUFFER, 0);
        spdlog::warn("Forcing reuse of UI sound source 0");
        return source;
    }
    
    return AL_NONE;
}

void UIAudioManager::releaseUISource(ALuint source) {
    alSourceStop(source);
    alSourcei(source, AL_BUFFER, 0);
} 