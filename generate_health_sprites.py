#!/usr/bin/env python3
"""
Health Bar Sprite Generator for Ortos II
Creates multiple versions of hands.png with different health bar levels
"""

import os
import sys
from PIL import Image, ImageDraw
import numpy as np

def analyze_image_structure(image_path):
    """Analyze the image to find the health bar area and extract the original red color"""
    try:
        img = Image.open(image_path)
        print(f"Image loaded: {img.size[0]}x{img.size[1]} pixels")
        print(f"Image mode: {img.mode}")
        
        # Convert to RGB if necessary
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        # Convert to numpy array for easier analysis
        img_array = np.array(img)
        
        # Find the right side of the image (assuming health bar is on the right)
        width = img_array.shape[1]
        right_portion = img_array[:, int(width * 0.7):]  # Take right 30% of image
        
        # Look for red pixels in the right portion
        red_pixels = np.where((right_portion[:, :, 0] > 150) & 
                             (right_portion[:, :, 1] < 100) & 
                             (right_portion[:, :, 2] < 100))
        
        original_red_color = None
        
        if len(red_pixels[0]) > 0:
            # Find bounding box of red area
            min_y, max_y = red_pixels[0].min(), red_pixels[0].max()
            min_x, max_x = red_pixels[1].min(), red_pixels[1].max()
            
            # Adjust coordinates to full image
            health_bar_x = int(width * 0.7) + min_x
            health_bar_y = min_y
            health_bar_width = max_x - min_x + 1
            health_bar_height = max_y - min_y + 1
            
            # Extract the original red color from the detected area
            red_area = img_array[health_bar_y:health_bar_y + health_bar_height, 
                               health_bar_x:health_bar_x + health_bar_width]
            
            # Find the most common red color in the area
            red_pixels_in_area = red_area[(red_area[:, :, 0] > 150) & 
                                         (red_area[:, :, 1] < 100) & 
                                         (red_area[:, :, 2] < 100)]
            
            if len(red_pixels_in_area) > 0:
                # Get the average red color
                original_red_color = tuple(np.mean(red_pixels_in_area, axis=0).astype(int))
                print(f"Extracted original red color: RGB{original_red_color}")
            
            print(f"Detected health bar area: x={health_bar_x}, y={health_bar_y}, w={health_bar_width}, h={health_bar_height}")
            return img, (health_bar_x, health_bar_y, health_bar_width, health_bar_height), original_red_color
        else:
            print("No red health bar detected. Using default area on the right side.")
            # Default to right side area
            health_bar_x = int(width * 0.8)
            health_bar_y = int(img_array.shape[0] * 0.1)
            health_bar_width = int(width * 0.15)
            health_bar_height = int(img_array.shape[0] * 0.8)
            return img, (health_bar_x, health_bar_y, health_bar_width, health_bar_height), None
            
    except Exception as e:
        print(f"Error analyzing image: {e}")
        return None, None, None

def create_health_variants(base_image, health_bar_area, output_dir, original_red_color):
    """Create 4 different health bar variants"""
    health_bar_x, health_bar_y, health_bar_width, health_bar_height = health_bar_area
    
    # Health levels: 100%, 75%, 50%, 25%
    health_levels = [1.0, 0.75, 0.5, 0.25]
    
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    for i, health_percentage in enumerate(health_levels):
        # Create a copy of the base image
        variant = base_image.copy()
        
        # Calculate the width of the red bar based on health percentage
        red_bar_width = int(health_bar_width * health_percentage)
        
        # Create a drawing context
        draw = ImageDraw.Draw(variant)
        
        # Clear the health bar area first (make it black/transparent)
        draw.rectangle([health_bar_x, health_bar_y, 
                       health_bar_x + health_bar_width, 
                       health_bar_y + health_bar_height], 
                      fill=(0, 0, 0, 0))  # Transparent black
        
        # Draw the red health bar
        if red_bar_width > 0:
            # Use original red color if available, otherwise use bright red
            red_color = original_red_color if original_red_color else (255, 0, 0)
            draw.rectangle([health_bar_x, health_bar_y, 
                           health_bar_x + red_bar_width, 
                           health_bar_y + health_bar_height], 
                          fill=red_color)
        
        # Save the variant
        filename = f"hands_health_{int(health_percentage * 100)}.png"
        filepath = os.path.join(output_dir, filename)
        variant.save(filepath)
        print(f"Created: {filepath} (Health: {int(health_percentage * 100)}%)")

def create_animated_health_variants(base_image, health_bar_area, output_dir, original_red_color):
    """Create more detailed health variants for smoother animation"""
    health_bar_x, health_bar_y, health_bar_width, health_bar_height = health_bar_area
    
    # More health levels for smoother animation: 100%, 80%, 60%, 40%, 20%, 0%
    health_levels = [1.0, 0.8, 0.6, 0.4, 0.2, 0.0]
    
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    for i, health_percentage in enumerate(health_levels):
        # Create a copy of the base image
        variant = base_image.copy()
        
        # Calculate the width of the red bar based on health percentage
        red_bar_width = int(health_bar_width * health_percentage)
        
        # Create a drawing context
        draw = ImageDraw.Draw(variant)
        
        # Clear the health bar area first (make it black/transparent)
        draw.rectangle([health_bar_x, health_bar_y, 
                       health_bar_x + health_bar_width, 
                       health_bar_y + health_bar_height], 
                      fill=(0, 0, 0, 0))  # Transparent black
        
        # Draw the red health bar with gradient effect
        if red_bar_width > 0:
            # Use original red color if available, otherwise use bright red
            base_red_color = original_red_color if original_red_color else (255, 0, 0)
            
            # Create a gradient from original red to slightly darker red
            for x in range(red_bar_width):
                # Calculate gradient intensity
                intensity = 1.0 - (x / red_bar_width) * 0.2  # Slight gradient
                red_value = int(base_red_color[0] * intensity)
                green_value = int(base_red_color[1] * intensity)
                blue_value = int(base_red_color[2] * intensity)
                
                draw.rectangle([health_bar_x + x, health_bar_y, 
                               health_bar_x + x + 1, 
                               health_bar_y + health_bar_height], 
                              fill=(red_value, green_value, blue_value))
        
        # Save the variant
        filename = f"hands_health_{int(health_percentage * 100)}.png"
        filepath = os.path.join(output_dir, filename)
        variant.save(filepath)
        print(f"Created: {filepath} (Health: {int(health_percentage * 100)}%)")

def main():
    # Paths
    input_image = "/Users/filipstupar/Documents/OrtosII/assets/graphic/enviroment/hands.png"
    output_dir = "/Users/filipstupar/Documents/OrtosII/assets/graphic/enviroment/health_variants"
    
    print("Health Bar Sprite Generator for Ortos II")
    print("=" * 50)
    
    # Check if input image exists
    if not os.path.exists(input_image):
        print(f"Error: Input image not found at {input_image}")
        return
    
    # Analyze the image structure
    print("Analyzing image structure...")
    base_image, health_bar_area, original_red_color = analyze_image_structure(input_image)
    
    if base_image is None or health_bar_area is None:
        print("Failed to analyze image structure.")
        return
    
    # Create basic health variants (4 levels)
    print("\nCreating basic health variants (4 levels)...")
    create_health_variants(base_image, health_bar_area, output_dir, original_red_color)
    
    # Create detailed health variants (6 levels for smoother animation)
    print("\nCreating detailed health variants (6 levels for animation)...")
    detailed_output_dir = os.path.join(output_dir, "detailed")
    create_animated_health_variants(base_image, health_bar_area, detailed_output_dir, original_red_color)
    
    print(f"\nAll variants created successfully in: {output_dir}")
    print("\nFiles created:")
    print("- hands_health_100.png (Full health)")
    print("- hands_health_75.png (75% health)")
    print("- hands_health_50.png (50% health)")
    print("- hands_health_25.png (25% health)")
    print("- detailed/ folder with 6 variants for smooth animation")

if __name__ == "__main__":
    main()
