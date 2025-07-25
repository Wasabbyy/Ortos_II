cmake_minimum_required(VERSION 3.10)
project(Ortos_II)

set(CMAKE_CXX_STANDARD 17)

# Find GLFW package
find_package(glfw3 REQUIRED)

# Find FreeType package
find_package(Freetype REQUIRED)

# Find GLM package
find_package(GLM REQUIRED)

# Find OpenAL package
if(APPLE)
    # On macOS, OpenAL is part of the system frameworks
    set(OPENAL_FOUND TRUE)
    set(OPENAL_INCLUDE_DIRS "")
    set(OPENAL_LIBRARIES "")
else()
    find_package(OpenAL REQUIRED)
endif()

# Include directories
include_directories(
    include
    src
    src/external
    ${GLFW_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIRS}
    # Add other include paths if needed
)

# Source files - explicitly list all source files
set(SOURCES
    src/main.cpp
    src/player/Player.cpp
    src/enemy/Enemy.cpp
    src/projectile/Projectile.cpp
    src/effects/BloodEffect.cpp
    src/audio/AudioManager.cpp
    src/audio/UIAudioManager.cpp
    src/ui/UI.cpp
    src/ui/TextRenderer.cpp
    src/input/InputHandler.cpp
    src/map/TileMap.cpp
    src/external/tinyxml2.cpp
    src/external/glad.c
    src/external/stb_image.cpp
    # Add other source files as needed
)

# Create executable
add_executable(Ortos_II ${SOURCES})

# Link libraries - platform specific handling
if(APPLE)
    target_link_libraries(Ortos_II
        glfw
        ${FREETYPE_LIBRARIES}
        ${GLM_LIBRARIES}
        ${OPENAL_LIBRARIES}
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenAL"
    )
elseif(UNIX AND NOT APPLE)  # Linux
    target_link_libraries(Ortos_II
        glfw
        ${FREETYPE_LIBRARIES}
        ${GLM_LIBRARIES}
        ${OPENAL_LIBRARIES}
        GL
        openal
    )
elseif(WIN32)  # Windows
    target_link_libraries(Ortos_II
        glfw
        ${FREETYPE_LIBRARIES}
        ${GLM_LIBRARIES}
        ${OPENAL_LIBRARIES}
        opengl32
        OpenAL32
    )
endif()