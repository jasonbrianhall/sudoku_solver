#!/usr/bin/env python3
import cv2
import numpy as np
import os
import argparse

def process_sudoku(input_path, output_dir):
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    # Load image and convert to grayscale
    image = cv2.imread(input_path)
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    
    # Apply adaptive thresholding
    thresh = cv2.adaptiveThreshold(gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, 
                                 cv2.THRESH_BINARY_INV, 57, 5)
    
    # Find contours
    cnts = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = cnts[0] if len(cnts) == 2 else cnts[1]
    
    # Find the largest contour - this should be our grid
    largest_contour = max(cnts, key=cv2.contourArea)
    
    # Get the bounding rectangle
    x, y, w, h = cv2.boundingRect(largest_contour)
    
    # Crop the image to the grid
    cropped = image[y:y+h, x:x+w]
    
    # Calculate cell dimensions from cropped image
    cell_height = h // 9
    cell_width = w // 9
    
    # Extract each cell from the cropped grid
    for i in range(9):
        for j in range(9):
            # Calculate cell coordinates
            y1 = i * cell_height
            y2 = (i + 1) * cell_height
            x1 = j * cell_width
            x2 = (j + 1) * cell_width
            
            # Extract cell
            cell = cropped[y1:y2, x1:x2]
            
            # Save cell
            filename = f"{output_dir}/cell_{i}_{j}.png"
            cv2.imwrite(filename, cell)

def main():
    parser = argparse.ArgumentParser(description='Crop Sudoku and split into cells')
    parser.add_argument('input_file', help='Input Sudoku image')
    parser.add_argument('output_dir', help='Output directory for cells')
    args = parser.parse_args()
    
    process_sudoku(args.input_file, args.output_dir)
    print(f"Extracted 81 cells to {args.output_dir}")

if __name__ == "__main__":
    main()
