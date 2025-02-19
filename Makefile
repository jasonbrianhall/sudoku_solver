.PHONY: all clean linux msdos python windows help

# Default target
all: linux msdos python windows
	@echo ""
	@echo "All builds complete except Windows Forms"
	@echo ""
	@echo "To build Windows Forms:"
	@echo ""
	@echo "Open SudokuSolver.vcxproj in Visual Studio"
	@echo "or run 'msbuild /p:Configuration=Release /p:Platform=x64 SudokuSolver.vcxproj' from a Visual Studio command prompt"

help:
	@echo "Available targets:"
	@echo "  help     - Show this help message"
	@echo "  all      - Build all components (except Windows Forms)"
	@echo "  linux    - Build Linux CLI version"
	@echo "  msdos    - Build MSDOS version"
	@echo "  python   - Build Python puzzle generators"
	@echo "  windows  - Build Windows CLI version"
	@echo "  clean    - Clean all build artifacts"
	@echo ""
	@echo "Windows Forms build instructions:"
	@echo "  Open SudokuSolver.vcxproj in Visual Studio"
	@echo "  or run 'msbuild /p:Configuration=Release /p:Platform=x64 SudokuSolver.vcxproj'"
	@echo "  from a Visual Studio command prompt"

linux:
	cd linux_cli && make

msdos:
	cd msdos && make

python:
	cd python_generate_puzzles && make

windows:
	cd windows_cli && make

clean:
	cd linux_cli && make clean
	cd msdos && make clean
	cd python_generate_puzzles && make clean
	cd windows_cli && make clean
