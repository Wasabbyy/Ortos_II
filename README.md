2D Roguelike Game – Bachelor Thesis Project
Project Overview

This project is the culmination of my Bachelor's thesis, where I designed and implemented a 2D roguelike game inspired by classics like The Binding of Isaac. Developed using C++ and OpenGL, the game blends procedural generation, permadeath mechanics, and action-combat gameplay into a rich, replayable experience.

The primary focus of this thesis was to explore core game development principles, including graphics rendering, collision detection, procedural content generation, and real-time input handling, all implemented from the ground up without using high-level game engines like Unity or Unreal.
 Features

    Real-Time Combat System: Fight off enemies with responsive controls and fluid animation.

    Permadeath: Death is permanent—start over each time you lose.

    Custom Tile-Based Engine: Engine built in C++ using OpenGL for rendering, with custom systems for entity management, tilemaps, and physics.

    Basic Audio System: Sound effects and background music using a lightweight audio library.

    Pixel Art Aesthetic: Simple, nostalgic graphics with a focus on clarity and retro appeal.

 Tech Stack

    Language: C++

    Graphics: OpenGL (with GLFW and GLEW)

    Audio: OpenAL or SDL_Mixer (based on configuration)

    Development Environment: Visual Studio

    Build System: CMake

 Gameplay Overview

    Start the Game: You spawn in the center of a dungeon level with a basic weapon.

    Explore: Move between rooms using keyboard controls.

    Combat: Defeat enemies using projectile attacks and dodging mechanics.

    Loot & Power-ups: Find health pickups, buffs, and secret items that affect your stats.

    Goal: Reach the final room and defeat the boss. Procedural layouts ensure no two runs are the same.

 Controls
Action	Key
Move Up	W
Move Down	S
Move Left	A
Move Right	D
Shoot (4 dirs)	Arrow Keys
Pause / Resume	P
Exit Game	ESC

Building the Project
Requirements

    C++17 compatible compiler (GCC, Clang, MSVC)

    OpenGL 3.3+

    CMake (or Make)

    GLEW / GLFW

    (Optional) OpenAL / SDL2 for sound

Build Instructions (Linux/macOS)

git clone https://github.com/Wasabbyy/BP_Game
cd BP_Game
./build_and_run.sh


Thesis Objectives

    Design and implement a minimal custom 2D game engine.

    Develop real-time game logic including AI, combat, and collision.

    Use OpenGL to manage sprite rendering, shaders, and visual effects.

    Explore the procedural generation of game content (map layout, item spawning, etc.).

    Optimize for performance while keeping codebase modular and maintainable.

    Reflect on the balance between game design theory and low-level implementation.
