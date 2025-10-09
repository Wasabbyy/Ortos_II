# Roman Numeral Level Display

## Overview
The game now displays player levels (1-100) using pixel art Roman numerals with a dungeon crawler theme featuring stone textures and lava cracks.

## Features Implemented

### 1. Pixel Art Generation
- **Script**: `create_roman_numerals.py`
- **Created Assets**:
  - `assets/graphic/roman_numerals/numeral_I.png` (36x96)
  - `assets/graphic/roman_numerals/numeral_V.png` (84x96)
  - `assets/graphic/roman_numerals/numeral_X.png` (84x96)
  - `assets/graphic/roman_numerals/numeral_C.png` (84x96)
- **Style**: Stone textures (dark, medium, light gray) with orange/red/yellow lava cracks
- **Format**: PNG with transparent background, 3x pixel art scaling

### 2. Roman Numeral Renderer
- **Files**: `src/ui/RomanNumeralRenderer.h` and `src/ui/RomanNumeralRenderer.cpp`
- **Features**:
  - Loads and manages Roman numeral textures
  - Converts numbers 1-100 to Roman numerals
  - Renders numerals with proper spacing
  - Handles subtractive notation (IV, IX, XL, XC, etc.)
  - Pixel-perfect nearest-neighbor filtering

### 3. UI Integration
- Updated `UI.h` and `UI.cpp` to include Roman numeral renderer
- Modified `drawLevelIndicator()` to display pixel art numerals
- Initialized in `GameInitializer.cpp` during UI setup
- Graceful fallback to text-based Roman numerals if renderer unavailable

### 4. Player Level System
- **Updated**: `src/player/Player.cpp`
- **Max Level**: Increased from 5 to 100
- **Level Display**: Shows correct Roman numerals for any level 1-100

## Roman Numeral Examples

| Level | Roman Numeral | Level | Roman Numeral |
|-------|---------------|-------|---------------|
| 1     | I             | 50    | L             |
| 2     | II            | 60    | LX            |
| 3     | III           | 70    | LXX           |
| 4     | IV            | 80    | LXXX          |
| 5     | V             | 90    | XC            |
| 10    | X             | 100   | C             |
| 20    | XX            | 40    | XL            |
| 30    | XXX           | 49    | XLIX          |

## Testing
To test different level displays:
1. Run the game
2. Use debug commands or cheats to change player level
3. Level indicator appears at the top-left of the screen
4. Roman numerals are displayed using the pixel art assets

## Regenerating Assets
To create new variations of the Roman numeral pixel art:
```bash
python3 create_roman_numerals.py
```
- Modify the `random.seed(42)` line for different lava crack patterns
- Adjust colors in the `COLORS` dictionary for different themes
- Change `scale` parameter for different sizes

## Technical Details
- **Rendering**: OpenGL immediate mode (GL_QUADS)
- **Blending**: Alpha blending enabled for transparent backgrounds
- **Texture Filtering**: GL_NEAREST for crisp pixel art
- **Coordinate System**: Screen space with orthographic projection
- **Position**: Top-left corner, next to health/XP bars

