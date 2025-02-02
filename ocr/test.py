#!/usr/bin/env python3
import cv2
import numpy as np
import argparse

def crop_grid(input_path, output_path):
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
    
    # Crop the image
    cropped = image[y:y+h, x:x+w]
    
    # Save the result
    cv2.imwrite(output_path, cropped)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input_file')
    parser.add_argument('output_file')
    args = parser.parse_args()
    
    crop_grid(args.input_file, args.output_file)

if __name__ == "__main__":
    main()
