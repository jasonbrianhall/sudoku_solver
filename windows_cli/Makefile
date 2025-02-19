CXX_WIN = x86_64-w64-mingw32-g++
CXX_FLAGS = -std=c++14
WIN_OUTPUT_DIR = win_sudoku_pdcurses
WIN_TARGET = $(WIN_OUTPUT_DIR)/sudoku_solver_pdcurses.exe
MINGW_BIN = /usr/x86_64-w64-mingw32/sys-root/mingw/bin

SOURCES = main.cpp sudoku.cpp generatepuzzle.cpp pdcursesprint.cpp
WIN_LIBS = -lpdcurses

.PHONY: all windows clean check-win-compiler

all: windows

windows: check-win-compiler
	@$(MAKE) $(WIN_TARGET)

$(WIN_TARGET): $(SOURCES)
	@mkdir -p $(WIN_OUTPUT_DIR)
	@echo "Compiling Windows version (PDCurses) ..."
	$(CXX_WIN) $(SOURCES) $(WIN_LIBS) $(CXX_FLAGS) -o $@
	@./collect_dlls.sh $@ $(MINGW_BIN) $(WIN_OUTPUT_DIR)

check-win-compiler:
	@which $(CXX_WIN) > /dev/null 2>&1 || \
		(echo "Windows cross-compiler not found - skipping Windows build" && exit 1)

clean:
	rm -rf $(WIN_OUTPUT_DIR)
