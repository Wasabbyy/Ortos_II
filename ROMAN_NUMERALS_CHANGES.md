# Roman Numerals - Improved Readability Update

## Changes Made

### 1. **Increased Size**
- **Old**: I: 36x96, V/X/C: 84x96
- **New**: I: 64x192, V/X/C: 160x192
- Result: ~1.8-2x larger assets for better readability

### 2. **Darker Stone Colors**
- **Old**: Grays ranging from 50-90
- **New**: Grays ranging from 35-70
- Result: Darker, more prominent numerals with better contrast

### 3. **More Prominent Lava Cracks**
- **Crack Density**: Increased from 8% to 12%
- **Brightness**: More bright/vibrant lava colors (40% bright vs 30%)
- Result: Lava cracks are now more visible and eye-catching

### 4. **Added Light Background**
- **New**: Subtle gray background (85,85,90 with 70% opacity) around numerals
- **Padding**: 2-pixel glow effect around the stone
- Result: Better readability without standing out too much

### 5. **Adjusted UI Scale**
- Changed from 0.5f to 0.4f
- Positioned 5 pixels lower for better alignment
- Result: Optimal display size on screen

## Visual Improvements

### Color Palette Updates
```
BEFORE:
- dark_stone: (50, 50, 55)
- stone: (70, 70, 75)
- light_stone: (90, 90, 95)
- lava_dark: (180, 50, 20)
- lava: (220, 80, 30)

AFTER:
- dark_stone: (35, 35, 40)    ⬇ Darker
- stone: (50, 50, 55)          ⬇ Darker
- light_stone: (65, 65, 70)    ⬇ Darker
- lava_dark: (200, 40, 10)     ⬆ More vibrant
- lava: (255, 70, 20)          ⬆ Brighter
- bg_light: (85, 85, 90, 180)  ✨ NEW
```

## How to Regenerate

If you want different variations:
```bash
python3 create_roman_numerals.py
```

Customization options in the script:
- `COLORS` dictionary - Adjust colors
- `crack_density` parameter - Change lava crack amount (default: 0.12)
- `padding` parameter - Adjust background glow size (default: 2)
- `random.seed(42)` - Change for different crack patterns

## Result

Roman numerals are now:
- ✅ Much more readable
- ✅ Better contrast against game background
- ✅ Darker with prominent lava details
- ✅ Subtle background glow for visibility
- ✅ Perfect size for in-game display

