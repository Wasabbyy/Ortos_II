#!/usr/bin/env python3
"""
Script to split xpbar.png into 11 separate XP bar images.
This script assumes the XP bars are arranged vertically in a single column.
"""

import os
from PIL import Image
import sys

def split_xpbar_image(input_path, output_dir):
    """
    Split the XP bar image into 11 separate images.
    
    Args:
        input_path (str): Path to the input xpbar.png file
        output_dir (str): Directory to save the split images
    """
    try:
        # Open the image
        with Image.open(input_path) as img:
            print(f"Original image size: {img.size}")
            print(f"Image mode: {img.mode}")
            
            # Get image dimensions
            width, height = img.size
            
            # Calculate the height of each XP bar (assuming 11 bars vertically)
            bar_height = height // 11
            
            print(f"Each XP bar will be {bar_height} pixels tall")
            
            # Create output directory if it doesn't exist
            os.makedirs(output_dir, exist_ok=True)
            
            # Split the image into 11 parts
            for i in range(11):
                # Calculate the top and bottom boundaries for this bar
                top = i * bar_height
                bottom = (i + 1) * bar_height
                
                # Crop the image
                cropped_img = img.crop((0, top, width, bottom))
                
                # Save the cropped image
                output_filename = f"xpbar_{i+1:02d}.png"
                output_path = os.path.join(output_dir, output_filename)
                cropped_img.save(output_path)
                
                print(f"Saved: {output_filename} ({cropped_img.size})")
            
            print(f"\nSuccessfully split {input_path} into 11 images!")
            print(f"Output directory: {output_dir}")
            
    except FileNotFoundError:
        print(f"Error: Could not find the file {input_path}")
        return False
    except Exception as e:
        print(f"Error processing image: {e}")
        return False
    
    return True

def main():
    # Define paths
    script_dir = os.path.dirname(os.path.abspath(__file__))
    input_path = os.path.join(script_dir, "assets", "graphic", "enviroment", "xpbar", "xpbar.png")
    output_dir = os.path.join(script_dir, "assets", "graphic", "enviroment", "xpbar", "split")
    
    print("XP Bar Image Splitter")
    print("=" * 30)
    print(f"Input file: {input_path}")
    print(f"Output directory: {output_dir}")
    print()
    
    # Check if input file exists
    if not os.path.exists(input_path):
        print(f"Error: Input file does not exist: {input_path}")
        print("Please make sure the xpbar.png file is in the correct location.")
        return 1
    
    # Split the image
    success = split_xpbar_image(input_path, output_dir)
    
    if success:
        print("\nDone! You can now use the individual XP bar images.")
        return 0
    else:
        return 1

if __name__ == "__main__":
    sys.exit(main())
