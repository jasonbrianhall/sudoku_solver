#!/usr/bin/env python

"""
Generate Sudoku puzzles with varying difficulties.

Example usage:
    # Generate 10 easy, 5 medium, 3 hard, and 2 extreme puzzles, saving to 'november_puzzles.docx':
    python generatepuzzles.py --easy 10 --medium 5 --hard 3 --extreme 2 --output november_puzzles.docx
    
    # Generate just 5 easy puzzles with default output filename (sudoku_puzzles.docx):
    python generatepuzzles.py --easy 5
    
    # Generate some medium and hard puzzles with custom filename:
    python generatepuzzles.py --medium 3 --hard 2 --output practice_puzzles.docx
"""

from sudoku_generator import SudokuPuzzleGenerator
import argparse

def main():
    # Create argument parser
    parser = argparse.ArgumentParser(
        description='Generate Sudoku puzzles of varying difficulties',
        formatter_class=argparse.RawTextHelpFormatter
    )
    
    # Add arguments for each difficulty level
    parser.add_argument('--easy', type=int, default=0,
                       help='Number of easy puzzles to generate')
    parser.add_argument('--medium', type=int, default=0,
                       help='Number of medium puzzles to generate')
    parser.add_argument('--hard', type=int, default=0,
                       help='Number of hard puzzles to generate')
    parser.add_argument('--extreme', type=int, default=0,
                       help='Number of extreme puzzles to generate')
    parser.add_argument('--output', type=str, default='sudoku_puzzles.docx',
                       help='Output filename (default: sudoku_puzzles.docx)')

    # Parse arguments
    args = parser.parse_args()

    # Create puzzle counts dictionary, excluding difficulties with 0 puzzles
    puzzle_counts = {}
    if args.easy > 0:
        puzzle_counts['easy'] = args.easy
    if args.medium > 0:
        puzzle_counts['medium'] = args.medium
    if args.hard > 0:
        puzzle_counts['hard'] = args.hard
    if args.extreme > 0:
        puzzle_counts['extreme'] = args.extreme

    # Check if at least one puzzle was requested
    if not puzzle_counts:
        parser.error("Please specify at least one puzzle to generate")

    try:
        # Create generator and generate puzzles
        generator = SudokuPuzzleGenerator()
        print(f"Generating puzzles:")
        for difficulty, count in puzzle_counts.items():
            print(f"  {count} {difficulty} puzzle(s)")
        
        generator.create_word_document(
            puzzle_counts=puzzle_counts,
            filename=args.output
        )
        
        print(f"\nPuzzles successfully generated and saved to: {args.output}")
        
    except Exception as e:
        print(f"Error generating puzzles: {str(e)}")
        exit(1)

if __name__ == '__main__':
    main()
