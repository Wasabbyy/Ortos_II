#!/usr/bin/env python3
"""
Verify that the transparency conversion worked correctly
"""

import os
from PIL import Image
import numpy as np

def verify_transparency(image_path):
    """Verify that black pixels have been made transparent"""
    try:
        img = Image.open(image_path)
        
        if img.mode != 'RGBA':
            print(f"❌ {os.path.basename(image_path)}: Not in RGBA mode")
            return False
        
        img_array = np.array(img)
        
        # Check for black pixels (RGB values all <= 30)
        black_pixels = (img_array[:, :, 0] <= 30) & \
                      (img_array[:, :, 1] <= 30) & \
                      (img_array[:, :, 2] <= 30)
        
        # Check if any black pixels are not transparent
        non_transparent_black = black_pixels & (img_array[:, :, 3] > 0)
        
        black_count = np.sum(black_pixels)
        non_transparent_count = np.sum(non_transparent_black)
        
        if non_transparent_count == 0:
            print(f"✅ {os.path.basename(image_path)}: All black pixels are transparent ({black_count} black pixels)")
            return True
        else:
            print(f"❌ {os.path.basename(image_path)}: {non_transparent_count} black pixels are not transparent")
            return False
            
    except Exception as e:
        print(f"❌ {os.path.basename(image_path)}: Error - {e}")
        return False

def main():
    """Verify all health variant images"""
    health_variants_dir = "/Users/filipstupar/Documents/OrtosII/assets/graphic/enviroment/health_variants"
    
    print("Verifying transparency conversion...")
    print("=" * 50)
    
    all_good = True
    
    # Check all PNG files in the directory
    for root, dirs, files in os.walk(health_variants_dir):
        for file in files:
            if file.lower().endswith('.png') and not file.endswith('.backup'):
                file_path = os.path.join(root, file)
                if not verify_transparency(file_path):
                    all_good = False
    
    print("=" * 50)
    if all_good:
        print("✅ All images have transparent black backgrounds!")
    else:
        print("❌ Some images still have non-transparent black pixels")

if __name__ == "__main__":
    main()

