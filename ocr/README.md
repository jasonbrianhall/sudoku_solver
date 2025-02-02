# Sudoku OCR

A Python tool that extracts Sudoku puzzles from images using Optical Character Recognition (OCR). The program processes an image of a Sudoku puzzle, detects the grid, extracts the numbers, and outputs them in a format compatible with Sudoku solvers.

## Features

- Image preprocessing for improved OCR accuracy
- Automatic grid detection using contour analysis
- Perspective correction for skewed images
- Digit extraction using Tesseract OCR
- Output format compatible with external Sudoku solvers
- Tested against sudoku.com, your miles may vary

## Prerequisites

The following Python packages are required:

```
opencv-python-headless  # opencv-python is known to break QT5 on Wayland
numpy
pytesseract
```

Additionally, you must have Tesseract OCR installed on your system:

- **Linux**: `sudo yum/apt-get install tesseract-ocr`
- **macOS**: `brew install tesseract`
- **Windows**: Download and install from the [official GitHub releases](https://github.com/UB-Mannheim/tesseract/wiki)

## Installation

1. Clone this repository:
   ```bash
   git clone <repository-url>
   cd sudoku-ocr
   ```

2. Install the required Python packages:
   ```bash
   pip install -r requirements.txt
   ```

## Usage

Run the script from the command line:

```bash
python sudoku_ocr.py <input_image> <output_file>
```

Arguments:
- `input_image`: Path to the image containing the Sudoku puzzle
- `output_file`: Path where the extracted puzzle should be saved

The output file will contain the Sudoku grid with dots (.) representing empty cells and numbers 1-9 for filled cells.

Example:
```bash
python sudoku_ocr.py puzzle.jpg extracted.txt
```

## How It Works

1. **Image Preprocessing**
   - Converts image to grayscale
   - Applies adaptive thresholding
   - Removes noise using morphological operations

2. **Grid Detection**
   - Finds contours in the preprocessed image
   - Identifies the largest contour (assumed to be the Sudoku grid)
   - Orders the corner points for perspective transformation

3. **Digit Extraction**
   - Applies perspective transform to get a top-down view
   - Divides the grid into 81 cells
   - Uses Tesseract OCR to recognize digits in each cell
   - Converts empty cells to 0 and builds the final grid

4. **Output Generation**
   - Converts the grid to the required format (dots for empty cells)
   - Saves the result to the specified output file

## Error Handling

The program includes error handling for common issues:
- Invalid image paths
- Failure to detect the grid
- Invalid grid corners
- OCR recognition failures

## Output Format

The extracted puzzle is saved in a text file with 9 rows, where:
- Each row contains 9 characters
- Numbers 1-9 represent filled cells
- Dots (.) represent empty cells
- Results are importable as .sud files

Example output:
```
.....2..3
..5..4...
........9
..1......
...3.....
.....7...
2........
...9..5..
4..8.....
```

## Limitations

- Requires clear, well-lit images of Sudoku puzzles
- Grid must be the prominent feature in the image
- Works best with printed puzzles (handwritten digits may have lower accuracy)
- Assumes traditional 9x9 Sudoku grid
- Accuracy isn't perfect so verify the results

