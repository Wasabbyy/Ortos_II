cmake_minimum_required(VERSION 3.10)
project(BP_Game)  # Make sure this matches your project name

set(CMAKE_CXX_STANDARD 17)

# Find GLFW
find_package(glfw3 REQUIRED)

# Include directories
include_directories(
    include
    ${GLFW_INCLUDE_DIRS}
)

# Source files
file(GLOB SOURCES "src/*.cpp" "src/glad.c")

# Create executable - MAKE SURE THIS MATCHES YOUR PROJECT NAME
add_executable(BP_Game ${SOURCES})  # Changed from OpenGLProject to BP_Game

# Link libraries
target_link_libraries(BP_Game
    glfw
    "-framework OpenGL"
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
)