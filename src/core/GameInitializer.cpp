#include "core/GameInitializer.h"
#include "audio/AudioManager.h"
#include "audio/UIAudioManager.h"
#include "ui/UI.h"
#include "projectile/Projectile.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <filesystem>
#include <stb_image.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

GameInitializer::GameInitializer() 
    : window(nullptr), windowWidth(1920), windowHeight(1080),
      audioManager(nullptr), uiAudioManager(nullptr),
      graphicsInitialized(false), audioInitialized(false), 
      uiInitialized(false), loggerInitialized(false) {
}

GameInitializer::~GameInitializer() {
    shutdown();
}

bool GameInitializer::initialize() {
    spdlog::info("=== Starting GameInitializer::initialize() ===");
    
    // Initialize in order: Logger -> Graphics -> UI -> Audio
    if (!initializeLogger()) {
        spdlog::error("Failed to initialize logger");
        return false;
    }
    
    if (!initializeGraphics(window)) {
        spdlog::error("Failed to initialize graphics");
        return false;
    }
    
    if (!initializeUI()) {
        spdlog::error("Failed to initialize UI");
        return false;
    }
    
    if (!initializeAudio()) {
        spdlog::error("Failed to initialize audio");
        return false;
    }
    
    spdlog::info("=== GameInitializer::initialize() completed successfully ===");
    return true;
}

bool GameInitializer::initializeLogger() {
    try {
        std::filesystem::create_directories("logs");
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/ortos.log", true);
        std::vector<spdlog::sink_ptr> sinks { console_sink, file_sink };
        auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::info);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        spdlog::info("Starting Ortos II application");
        loggerInitialized = true;
        return true;
    } catch (const std::exception& e) {
        // Fallback to default logger if file sink fails
        spdlog::set_level(spdlog::level::debug);
        spdlog::warn("Failed to initialize file logger: {}", e.what());
        loggerInitialized = false;
        return false;
    }
}

bool GameInitializer::initializeGraphics(GLFWwindow*& window) {
    spdlog::info("Initializing graphics system...");
    
    if (!glfwInit()) {
        spdlog::error("Failed to initialize GLFW");
        return false;
    }

    // Get primary monitor and create fullscreen window
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    window = glfwCreateWindow(mode->width, mode->height, "Ortos II", primaryMonitor, nullptr);

    if (!window) {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    // Set window properties
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
    glfwMakeContextCurrent(window);
    
    // Setup OpenGL state
    if (!setupOpenGL()) {
        return false;
    }
    
    // Set up viewport and projection
    glfwSetWindowSize(window, windowWidth, windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);
    setupProjection();
    
    this->window = window;
    graphicsInitialized = true;
    spdlog::info("Graphics system initialized successfully");
    return true;
}

bool GameInitializer::setupOpenGL() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); 
    glEnable(GL_TEXTURE_2D);
    
    // Set up input callbacks
    setupInputCallbacks();
    
    return true;
}

bool GameInitializer::setupProjection() {
    // Set up orthographic projection for 2D rendering
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    return true;
}

void GameInitializer::setupInputCallbacks() {
    if (!window) return;
    
    // Set up any input callbacks here if needed
    // Currently using direct key polling in main loop
}

bool GameInitializer::initializeUI() {
    spdlog::info("Initializing UI system...");
    
    // Initialize UI system with FreeType
    if (!UI::init(getAssetPath("assets/fonts/pixel.ttf"))) {
        spdlog::error("Failed to initialize UI system");
        return false;
    }

    // Load title screen background texture
    if (!UI::loadTitleScreenTexture(getAssetPath("assets/screens/titlescreen.png"))) {
        spdlog::warn("Failed to load title screen texture, will use black background");
    }

    // Load death screen background texture
    if (!UI::loadDeathScreenTexture(getAssetPath("assets/screens/deathscreen.png"))) {
        spdlog::warn("Failed to load death screen texture, will use black background");
    }
    
    // Initialize animated health bar
    UI::initAnimatedHealthBar(getAssetPath(""));
    
    // Initialize animated XP bar
    UI::initAnimatedXPBar(getAssetPath(""));
    
    // Initialize Roman numeral renderer for level display
    UI::initRomanNumeralRenderer(getAssetPath("assets/graphic/roman_numerals"));
    
    // Load all projectile textures (player, eye, shroom)
    Projectile::loadAllProjectileTextures();
    
    uiInitialized = true;
    spdlog::info("UI system initialized successfully");
    return true;
}

bool GameInitializer::initializeAudio() {
    spdlog::info("Initializing audio system...");
    
    // Initialize AudioManager
    audioManager = std::make_unique<AudioManager>();
    spdlog::info("Attempting to initialize AudioManager...");
    if (!audioManager->init()) {
        spdlog::error("Failed to initialize AudioManager");
        return false;
    }
    spdlog::info("AudioManager initialized successfully");

    // Initialize UIAudioManager
    uiAudioManager = std::make_unique<UIAudioManager>();
    spdlog::info("Attempting to initialize UIAudioManager...");
    if (!uiAudioManager->init(audioManager->getContext())) {
        spdlog::error("Failed to initialize UIAudioManager");
        return false;
    }
    spdlog::info("UIAudioManager initialized successfully");

    // Load UI sound effects
    spdlog::info("Attempting to load UI sound effects...");
    if (!uiAudioManager->loadUISound("button", getAssetPath("assets/sounds/button.wav"))) {
        spdlog::warn("Failed to load button sound");
    } else {
        spdlog::info("Successfully loaded button sound");
    }

    // Load intro music for title screen
    spdlog::info("Attempting to load intro music...");
    if (!audioManager->loadMusic("intro", getAssetPath("assets/sounds/intro.wav"))) {
        spdlog::warn("Failed to load intro music");
    } else {
        spdlog::info("Successfully loaded intro music");
    }

    // Load background music for gameplay
    spdlog::info("Attempting to load background music...");
    if (!audioManager->loadMusic("background", getAssetPath("assets/sounds/defaultSong.wav"))) {
        spdlog::warn("Failed to load background music");
    } else {
        spdlog::info("Successfully loaded background music");
    }
    
    audioInitialized = true;
    spdlog::info("Audio system initialized successfully");
    return true;
}

void GameInitializer::shutdown() {
    spdlog::info("Shutting down GameInitializer...");
    
    if (uiInitialized) {
        UI::cleanup();
        uiInitialized = false;
    }
    
    if (audioInitialized) {
        uiAudioManager.reset();
        audioManager.reset();
        audioInitialized = false;
    }
    
    if (graphicsInitialized) {
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
        graphicsInitialized = false;
    }
    
    spdlog::info("GameInitializer shutdown completed");
}

std::string GameInitializer::getAssetPath(const std::string& relativePath) {
    // First, try to get the actual executable path
    std::filesystem::path executablePath;
    
#ifdef __APPLE__
    // macOS-specific executable path detection
    char buffer[1024];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        executablePath = std::filesystem::path(buffer).parent_path();
    }
#else
    // Linux-specific executable path detection
    try {
        executablePath = std::filesystem::canonical("/proc/self/exe");
    } catch (const std::filesystem::filesystem_error& e) {
        // Fallback if /proc/self/exe is not available
        executablePath = std::filesystem::path();
    }
#endif
    
    // If we got the executable path, use it to find the project root
    if (!executablePath.empty()) {
        // If executable is in build directory, go up one level to project root
        if (executablePath.filename() == "build") {
            executablePath = executablePath.parent_path();
        }
        
        // Check if assets directory exists relative to executable
        if (std::filesystem::exists(executablePath / "assets")) {
            std::string result = (executablePath / relativePath).string();
            return result;
        }
        
        // Try going up from executable directory
        for (int i = 0; i < 5; i++) {
            executablePath = executablePath.parent_path();
            if (std::filesystem::exists(executablePath / "assets")) {
                return (executablePath / relativePath).string();
            }
        }
    }
    
    // Fallback: try current working directory
    std::filesystem::path currentPath = std::filesystem::current_path();
    
    // Check if we're already in the project root (assets directory exists)
    if (std::filesystem::exists(currentPath / "assets")) {
        return (currentPath / relativePath).string();
    }
    
    // If we're in the build directory, go up one level
    if (currentPath.filename() == "build") {
        currentPath = currentPath.parent_path();
        if (std::filesystem::exists(currentPath / "assets")) {
            return (currentPath / relativePath).string();
        }
    }
    
    // Try going up directories to find the project root
    for (int i = 0; i < 10; i++) {
        currentPath = currentPath.parent_path();
        if (std::filesystem::exists(currentPath / "assets")) {
            return (currentPath / relativePath).string();
        }
    }
    
    // Last resort: try common project locations
    std::vector<std::string> commonPaths = {
        "/Users/filipstupar/Documents/OrtosII",
        "./",
        "../",
        "../../",
        "../../../"
    };
    
    for (const auto& path : commonPaths) {
        std::filesystem::path testPath(path);
        if (std::filesystem::exists(testPath / "assets")) {
            return (testPath / relativePath).string();
        }
    }
    
    // If all else fails, return the relative path as-is
    spdlog::warn("Could not find assets directory, using relative path: {}", relativePath);
    return relativePath;
}
