#!/usr/bin/env python3
"""
Health Variants Transparency Converter for Ortos II
Makes black backgrounds transparent in health variant images
"""

import os
import sys
from PIL import Image
import numpy as np

def make_black_transparent(image_path, output_path=None, tolerance=30):
    """
    Convert black pixels to transparent in an image
    
    Args:
        image_path (str): Path to the input image
        output_path (str): Path to save the output image (if None, overwrites original)
        tolerance (int): Tolerance for considering a pixel as "black" (0-255)
    
    Returns:
        bool: True if successful, False otherwise
    """
    try:
        # Open the image
        img = Image.open(image_path)
        print(f"Processing: {os.path.basename(image_path)} ({img.size[0]}x{img.size[1]})")
        
        # Convert to RGBA if not already
        if img.mode != 'RGBA':
            img = img.convert('RGBA')
        
        # Convert to numpy array for easier manipulation
        img_array = np.array(img)
        
        # Create a mask for black pixels (with tolerance)
        # A pixel is considered "black" if all RGB values are below the tolerance
        black_mask = (img_array[:, :, 0] <= tolerance) & \
                    (img_array[:, :, 1] <= tolerance) & \
                    (img_array[:, :, 2] <= tolerance)
        
        # Count black pixels
        black_pixel_count = np.sum(black_mask)
        total_pixels = img_array.shape[0] * img_array.shape[1]
        black_percentage = (black_pixel_count / total_pixels) * 100
        
        print(f"  Found {black_pixel_count} black pixels ({black_percentage:.1f}% of image)")
        
        # Make black pixels transparent
        img_array[black_mask, 3] = 0  # Set alpha channel to 0 (transparent)
        
        # Convert back to PIL Image
        result_img = Image.fromarray(img_array, 'RGBA')
        
        # Save the result
        if output_path is None:
            output_path = image_path
        
        result_img.save(output_path)
        print(f"  Saved: {os.path.basename(output_path)}")
        
        return True
        
    except Exception as e:
        print(f"Error processing {image_path}: {e}")
        return False

def process_health_variants_directory(directory_path, tolerance=30, create_backup=True):
    """
    Process all PNG images in the health variants directory
    
    Args:
        directory_path (str): Path to the health variants directory
        tolerance (int): Tolerance for black pixel detection
        create_backup (bool): Whether to create backup copies before processing
    """
    if not os.path.exists(directory_path):
        print(f"Error: Directory not found: {directory_path}")
        return False
    
    # Find all PNG files in the directory and subdirectories
    png_files = []
    for root, dirs, files in os.walk(directory_path):
        for file in files:
            if file.lower().endswith('.png'):
                png_files.append(os.path.join(root, file))
    
    if not png_files:
        print(f"No PNG files found in {directory_path}")
        return False
    
    print(f"Found {len(png_files)} PNG files to process")
    print(f"Using black tolerance: {tolerance}")
    print("=" * 60)
    
    successful = 0
    failed = 0
    
    for png_file in png_files:
        # Create backup if requested
        if create_backup:
            backup_path = png_file + '.backup'
            if not os.path.exists(backup_path):
                try:
                    import shutil
                    shutil.copy2(png_file, backup_path)
                    print(f"  Created backup: {os.path.basename(backup_path)}")
                except Exception as e:
                    print(f"  Warning: Could not create backup: {e}")
        
        # Process the image
        if make_black_transparent(png_file, tolerance=tolerance):
            successful += 1
        else:
            failed += 1
        
        print()  # Empty line for readability
    
    print("=" * 60)
    print(f"Processing complete!")
    print(f"Successfully processed: {successful} files")
    print(f"Failed: {failed} files")
    
    if create_backup:
        print(f"\nBackup files created with .backup extension")
        print("You can delete them once you're satisfied with the results")
    
    return failed == 0

def main():
    """Main function"""
    print("Health Variants Transparency Converter for Ortos II")
    print("=" * 60)
    
    # Path to the health variants directory
    health_variants_dir = "/Users/filipstupar/Documents/OrtosII/assets/graphic/enviroment/health_variants"
    
    # Check if directory exists
    if not os.path.exists(health_variants_dir):
        print(f"Error: Health variants directory not found: {health_variants_dir}")
        print("Please make sure the directory exists and contains PNG files.")
        return
    
    # Process all images in the directory
    success = process_health_variants_directory(
        health_variants_dir, 
        tolerance=30,  # Adjust this value if needed (0-255)
        create_backup=True
    )
    
    if success:
        print("\n✅ All images processed successfully!")
        print("Black backgrounds have been made transparent.")
    else:
        print("\n❌ Some images failed to process. Check the error messages above.")

if __name__ == "__main__":
    main()

