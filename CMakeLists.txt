cmake_minimum_required(VERSION 3.10)
project(BP_Game)

set(CMAKE_CXX_STANDARD 17)

# Find GLFW package
find_package(glfw3 REQUIRED)

# Include directories
include_directories(
    include
    ${GLFW_INCLUDE_DIRS}
    # Add other include paths if needed
)

# Source files - explicitly list all source files
set(SOURCES
    src/Application.cpp
    src/Player.cpp
    src/InputHandler.cpp
    src/glad.c
    src/stb_image.cpp
    # Add other source files as needed
)

# Create executable
add_executable(BP_Game ${SOURCES})

# Link libraries - platform specific handling
if(APPLE)
    target_link_libraries(BP_Game
        glfw
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
elseif(UNIX AND NOT APPLE)  # Linux
    target_link_libraries(BP_Game
        glfw
        GL
    )
elseif(WIN32)  # Windows
    target_link_libraries(BP_Game
        glfw
        opengl32
    )
endif()