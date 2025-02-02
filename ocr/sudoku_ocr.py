import cv2
import numpy as np
import pytesseract
from typing import List, Tuple

class SudokuOCR:
    def __init__(self, image_path: str):
        self.image = cv2.imread(image_path)
        if self.image is None:
            raise ValueError(f"Could not load image from {image_path}")
        self.gray = cv2.cvtColor(self.image, cv2.COLOR_BGR2GRAY)
        self.height, self.width = self.gray.shape

    def preprocess_image(self) -> np.ndarray:
        """Preprocess the image for better OCR results."""
        # Apply adaptive thresholding
        binary = cv2.adaptiveThreshold(
            self.gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, 
            cv2.THRESH_BINARY_INV, 11, 2
        )
        
        # Remove noise
        kernel = np.ones((2, 2), np.uint8)
        binary = cv2.morphologyEx(binary, cv2.MORPH_OPEN, kernel)
        
        return binary

    def find_grid(self, preprocessed: np.ndarray) -> np.ndarray:
        """Find and return the corners of the Sudoku grid."""
        # Find contours
        contours, _ = cv2.findContours(
            preprocessed, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
        )
        
        # Find the largest contour (should be the Sudoku grid)
        max_area = 0
        grid_contour = None
        
        for contour in contours:
            area = cv2.contourArea(contour)
            if area > max_area:
                max_area = area
                grid_contour = contour
        
        if grid_contour is None:
            raise ValueError("Could not find Sudoku grid in image")
        
        # Approximate the contour to get a rectangle
        peri = cv2.arcLength(grid_contour, True)
        approx = cv2.approxPolyDP(grid_contour, 0.02 * peri, True)
        
        if len(approx) != 4:
            raise ValueError("Could not find four corners of Sudoku grid")
            
        return self.order_points(approx.reshape(4, 2))

    def order_points(self, pts: np.ndarray) -> np.ndarray:
        """Order points in [top-left, top-right, bottom-right, bottom-left] order."""
        rect = np.zeros((4, 2), dtype=np.float32)
        
        # Top-left will have smallest sum
        # Bottom-right will have largest sum
        s = pts.sum(axis=1)
        rect[0] = pts[np.argmin(s)]
        rect[2] = pts[np.argmax(s)]
        
        # Top-right will have smallest difference
        # Bottom-left will have largest difference
        diff = np.diff(pts, axis=1)
        rect[1] = pts[np.argmin(diff)]
        rect[3] = pts[np.argmax(diff)]
        
        return rect

    def perspective_transform(self, corners: np.ndarray) -> np.ndarray:
        """Apply perspective transform to get a top-down view of the grid."""
        # Calculate dimensions
        width = int(max(
            np.linalg.norm(corners[1] - corners[0]),
            np.linalg.norm(corners[2] - corners[3])
        ))
        height = int(max(
            np.linalg.norm(corners[3] - corners[0]),
            np.linalg.norm(corners[2] - corners[1])
        ))
        
        # Define destination points
        dst = np.array([
            [0, 0],
            [width - 1, 0],
            [width - 1, height - 1],
            [0, height - 1]
        ], dtype=np.float32)
        
        # Calculate and apply transform
        matrix = cv2.getPerspectiveTransform(corners, dst)
        return cv2.warpPerspective(self.gray, matrix, (width, height))

    def extract_digits(self, warped: np.ndarray) -> List[List[int]]:
        """Extract digits from the grid using OCR."""
        cell_height = warped.shape[0] // 9
        cell_width = warped.shape[1] // 9
        grid = []
        
        for i in range(9):
            row = []
            for j in range(9):
                # Extract cell
                cell = warped[
                    i * cell_height:(i + 1) * cell_height,
                    j * cell_width:(j + 1) * cell_width
                ]
                
                # Add padding
                padding = 5
                cell = cell[padding:-padding, padding:-padding]
                
                # Preprocess cell
                cell = cv2.threshold(
                    cell, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU
                )[1]
                
                # OCR
                config = '--psm 10 --oem 3 -c tessedit_char_whitelist=123456789'
                digit = pytesseract.image_to_string(
                    cell, config=config
                ).strip()
                
                # Convert to integer or 0 if empty
                row.append(int(digit) if digit.isdigit() else 0)
            
            grid.append(row)
        
        return grid

    def process(self) -> List[List[int]]:
        """Process the image and return the extracted Sudoku grid."""
        preprocessed = self.preprocess_image()
        corners = self.find_grid(preprocessed)
        warped = self.perspective_transform(corners)
        return self.extract_digits(warped)

def save_grid(grid: List[List[int]], filename: str):
    """Save grid in format compatible with C++ solver."""
    with open(filename, 'w') as f:
        for row in grid:
            # Convert numbers to string, replacing 0s with dots
            line = ''.join(str(x) if x != 0 else '.' for x in row)
            f.write(line + '\n')

def main(image_path: str, output_path: str):
    """Extract Sudoku puzzle from image and save in solver format."""
    try:
        # Initialize OCR
        ocr = SudokuOCR(image_path)
        
        # Extract grid
        print("Extracting Sudoku grid from image...")
        grid = ocr.process()
        
        # Save in solver format
        print(f"\nSaving grid to {output_path}")
        save_grid(grid, output_path)
        
        # Display extracted grid
        print("\nExtracted grid:")
        for row in grid:
            print(''.join(str(x) if x != 0 else '.' for x in row))
            
    except Exception as e:
        print(f"Error: {str(e)}")

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_image> <output_file>")
        sys.exit(1)
        
    main(sys.argv[1], sys.argv[2])
