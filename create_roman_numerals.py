#!/usr/bin/env python3
"""
Generate pixel art Roman numerals (I, V, X, C) with dungeon/stone theme and lava cracks.
These can be used for level indicators from 1-100.
"""

from PIL import Image, ImageDraw
import random
import os

# Color palette - stone/dungeon theme with lava
COLORS = {
    'dark_stone': (35, 35, 40),      # Darker stone
    'stone': (50, 50, 55),            # Darker medium stone
    'light_stone': (65, 65, 70),      # Darker light stone
    'lava_dark': (200, 40, 10),       # More vibrant dark lava
    'lava': (255, 70, 20),            # Brighter lava
    'lava_bright': (255, 140, 50),    # Bright lava
    'lava_glow': (255, 200, 100),     # Glowing lava
    'bg_light': (85, 85, 90, 180),    # Subtle light background (semi-transparent)
    'transparent': (0, 0, 0, 0)
}

def add_stone_texture(pixels, width, height, base_pattern):
    """Add stone texture variation to the base pattern."""
    for y in range(height):
        for x in range(width):
            if base_pattern[y][x] == 1:  # Stone area
                # Random stone color variation (more dark stones for better contrast)
                rand = random.random()
                if rand < 0.5:
                    pixels[x, y] = COLORS['dark_stone']
                elif rand < 0.85:
                    pixels[x, y] = COLORS['stone']
                else:
                    pixels[x, y] = COLORS['light_stone']

def add_lava_cracks(pixels, width, height, base_pattern, crack_density=0.12):
    """Add lava crack details to the stone."""
    for y in range(height):
        for x in range(width):
            if base_pattern[y][x] == 1:  # Only on stone areas
                if random.random() < crack_density:
                    # Create lava crack (more prominent)
                    rand = random.random()
                    if rand < 0.4:
                        pixels[x, y] = COLORS['lava_bright']
                    elif rand < 0.7:
                        pixels[x, y] = COLORS['lava']
                    else:
                        pixels[x, y] = COLORS['lava_dark']

def add_background(pixels, width, height, base_pattern, padding=2):
    """Add a subtle light background behind the numeral for better readability."""
    for y in range(height):
        for x in range(width):
            if base_pattern[y][x] == 0:  # Empty area
                # Check if near stone (within padding pixels)
                has_neighbor = False
                for dy in range(-padding, padding + 1):
                    for dx in range(-padding, padding + 1):
                        ny, nx = y + dy, x + dx
                        if 0 <= ny < height and 0 <= nx < width:
                            if base_pattern[ny][nx] == 1:
                                has_neighbor = True
                                break
                    if has_neighbor:
                        break
                
                if has_neighbor:
                    pixels[x, y] = COLORS['bg_light']

def create_numeral_i():
    """Create Roman numeral I (vertical bar with thickness)."""
    width, height = 16, 48  # Increased size
    scale = 4  # Larger scale
    
    # Define the pattern (0 = transparent, 1 = stone)
    pattern = []
    for y in range(height):
        row = []
        for x in range(width):
            # Create vertical bar in the middle with some width
            if 4 <= x <= 7:
                row.append(1)
            else:
                row.append(0)
        pattern.append(row)
    
    # Create image
    img = Image.new('RGBA', (width * scale, height * scale), COLORS['transparent'])
    
    # Create at 1x size first
    temp_img = Image.new('RGBA', (width, height), COLORS['transparent'])
    pixels = temp_img.load()
    
    add_background(pixels, width, height, pattern, padding=2)
    add_stone_texture(pixels, width, height, pattern)
    add_lava_cracks(pixels, width, height, pattern)
    
    # Scale up with nearest neighbor to maintain pixel art look
    img = temp_img.resize((width * scale, height * scale), Image.NEAREST)
    
    return img

def create_numeral_v():
    """Create Roman numeral V."""
    width, height = 40, 48  # Increased size
    scale = 4  # Larger scale
    
    pattern = []
    for y in range(height):
        row = []
        for x in range(width):
            # V shape - two diagonal lines meeting at bottom
            # Left diagonal
            left_diag = abs(x - (4 + y * 0.3))
            # Right diagonal  
            right_diag = abs(x - (24 - y * 0.3))
            
            if y < height - 2:
                if left_diag < 2.5 or right_diag < 2.5:
                    row.append(1)
                else:
                    row.append(0)
            else:
                # Bottom point
                if 12 <= x <= 16:
                    row.append(1)
                else:
                    row.append(0)
        pattern.append(row)
    
    temp_img = Image.new('RGBA', (width, height), COLORS['transparent'])
    pixels = temp_img.load()
    
    add_background(pixels, width, height, pattern, padding=2)
    add_stone_texture(pixels, width, height, pattern)
    add_lava_cracks(pixels, width, height, pattern)
    
    img = temp_img.resize((width * scale, height * scale), Image.NEAREST)
    
    return img

def create_numeral_x():
    """Create Roman numeral X."""
    width, height = 40, 48  # Increased size
    scale = 4  # Larger scale
    
    pattern = []
    for y in range(height):
        row = []
        for x in range(width):
            # X shape - two diagonals crossing
            # Diagonal from top-left to bottom-right
            diag1 = abs((x - 4) - (y * 0.7))
            # Diagonal from top-right to bottom-left
            diag2 = abs((x - 24) + (y * 0.7))
            
            if diag1 < 2.5 or diag2 < 2.5:
                row.append(1)
            else:
                row.append(0)
        pattern.append(row)
    
    temp_img = Image.new('RGBA', (width, height), COLORS['transparent'])
    pixels = temp_img.load()
    
    add_background(pixels, width, height, pattern, padding=2)
    add_stone_texture(pixels, width, height, pattern)
    add_lava_cracks(pixels, width, height, pattern)
    
    img = temp_img.resize((width * scale, height * scale), Image.NEAREST)
    
    return img

def create_numeral_c():
    """Create Roman numeral C (curved arc opening to the right)."""
    width, height = 40, 48  # Increased size
    scale = 4  # Larger scale
    
    pattern = []
    for y in range(height):
        row = []
        for x in range(width):
            # C shape - arc opening to the right
            center_x, center_y = 16, 16
            distance = ((x - center_x) ** 2 + (y - center_y) ** 2) ** 0.5
            angle_rad = 0 if x == center_x else abs((y - center_y) / (x - center_x + 0.001))
            
            # Create arc
            if 10 < distance < 14 and x < center_x + 2:
                row.append(1)
            # Top and bottom extensions
            elif y < 8 and 10 < x < 18 and 2 < y:
                row.append(1)
            elif y > 24 and 10 < x < 18 and y < 30:
                row.append(1)
            else:
                row.append(0)
        pattern.append(row)
    
    temp_img = Image.new('RGBA', (width, height), COLORS['transparent'])
    pixels = temp_img.load()
    
    add_background(pixels, width, height, pattern, padding=2)
    add_stone_texture(pixels, width, height, pattern)
    add_lava_cracks(pixels, width, height, pattern)
    
    img = temp_img.resize((width * scale, height * scale), Image.NEAREST)
    
    return img

def main():
    """Generate all Roman numeral images."""
    # Set random seed for reproducibility (remove or change for different variations)
    random.seed(42)
    
    # Create output directory
    output_dir = 'assets/graphic/roman_numerals'
    os.makedirs(output_dir, exist_ok=True)
    
    print("Generating pixel art Roman numerals...")
    
    # Generate each numeral
    numerals = {
        'I': create_numeral_i(),
        'V': create_numeral_v(),
        'X': create_numeral_x(),
        'C': create_numeral_c()
    }
    
    # Save images
    for name, img in numerals.items():
        filename = f'{output_dir}/numeral_{name}.png'
        img.save(filename)
        print(f"Created: {filename} ({img.width}x{img.height})")
    
    print("\nDone! Roman numerals created successfully.")
    print(f"Files saved to: {output_dir}/")
    print("\nYou can now use these to compose level indicators from 1-100:")
    print("  Examples: I=1, II=2, III=3, IV=4, V=5, VI=6, ...")
    print("            X=10, XX=20, XXX=30, XL=40, L=50, ...")
    print("            C=100")

if __name__ == '__main__':
    main()

