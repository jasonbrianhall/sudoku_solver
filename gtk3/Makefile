# Compiler settings
CXX_LINUX = g++
CXX_WIN = x86_64-w64-mingw32-gcc
CFLAGS_COMMON = -std=c++11 -Wall -O2

# Debug flags
DEBUG_FLAGS = -g -DDEBUG

# Object files
OBJS_COMMON = sudoku.o debug.o generatepuzzle.o sudoku_game.o

# Target executables
TARGET_LINUX = sudoku_game
TARGET_WIN = sudoku_game.exe
TARGET_LINUX_DEBUG = sudoku_game_debug
TARGET_WIN_DEBUG = sudoku_game_debug.exe

# Build directories
BUILD_DIR = build
BUILD_DIR_LINUX = $(BUILD_DIR)/linux
BUILD_DIR_WIN = $(BUILD_DIR)/windows
BUILD_DIR_LINUX_DEBUG = $(BUILD_DIR)/linux_debug
BUILD_DIR_WIN_DEBUG = $(BUILD_DIR)/windows_debug

# Windows DLL settings
DLL_SOURCE_DIR = /usr/x86_64-w64-mingw32/sys-root/mingw/bin

# Create necessary directories
$(shell mkdir -p $(BUILD_DIR_LINUX) $(BUILD_DIR_WIN) $(BUILD_DIR_LINUX_DEBUG) $(BUILD_DIR_WIN_DEBUG))

# Use pkg-config for dependencies
GTK_CFLAGS_LINUX := $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS_LINUX := $(shell pkg-config --libs gtk+-3.0)
GTK_CFLAGS_WIN := $(shell mingw64-pkg-config --cflags gtk+-3.0)
GTK_LIBS_WIN := $(shell mingw64-pkg-config --libs gtk+-3.0)

# Platform-specific settings
CFLAGS_LINUX = $(CFLAGS_COMMON) $(GTK_CFLAGS_LINUX)
CFLAGS_WIN = $(CFLAGS_COMMON) $(GTK_CFLAGS_WIN)

# Debug-specific flags
CFLAGS_LINUX_DEBUG = $(CFLAGS_LINUX) $(DEBUG_FLAGS)
CFLAGS_WIN_DEBUG = $(CFLAGS_WIN) $(DEBUG_FLAGS)

LDFLAGS_LINUX = $(GTK_LIBS_LINUX)
LDFLAGS_WIN = $(GTK_LIBS_WIN) -lstdc++ -mwindows

# Object files with build path
OBJS_LINUX = $(addprefix $(BUILD_DIR_LINUX)/,$(OBJS_COMMON))
OBJS_WIN = $(addprefix $(BUILD_DIR_WIN)/,$(patsubst %.o,%.win.o,$(OBJS_COMMON)))
OBJS_LINUX_DEBUG = $(addprefix $(BUILD_DIR_LINUX_DEBUG)/,$(patsubst %.o,%.debug.o,$(OBJS_COMMON)))
OBJS_WIN_DEBUG = $(addprefix $(BUILD_DIR_WIN_DEBUG)/,$(patsubst %.o,%.win.debug.o,$(OBJS_COMMON)))

# Default target
.PHONY: all
all: linux windows

# OS-specific builds
.PHONY: linux
linux: $(BUILD_DIR_LINUX)/$(TARGET_LINUX)

.PHONY: windows
windows: $(BUILD_DIR_WIN)/$(TARGET_WIN) collect-dlls

# Debug builds
.PHONY: linux-debug
linux-debug: $(BUILD_DIR_LINUX_DEBUG)/$(TARGET_LINUX_DEBUG)

.PHONY: windows-debug
windows-debug: $(BUILD_DIR_WIN_DEBUG)/$(TARGET_WIN_DEBUG) collect-debug-dlls

# Build all debug versions
.PHONY: all-debug
all-debug: linux-debug windows-debug

# Main Linux target
$(BUILD_DIR_LINUX)/$(TARGET_LINUX): $(OBJS_LINUX)
	$(CXX_LINUX) $(CFLAGS_LINUX) -o $@ $^ $(LDFLAGS_LINUX)

# Main Windows target
$(BUILD_DIR_WIN)/$(TARGET_WIN): $(OBJS_WIN)
	$(CXX_WIN) $(CFLAGS_WIN) -o $@ $^ $(LDFLAGS_WIN)

# Debug Linux target
$(BUILD_DIR_LINUX_DEBUG)/$(TARGET_LINUX_DEBUG): $(OBJS_LINUX_DEBUG)
	$(CXX_LINUX) $(CFLAGS_LINUX_DEBUG) -o $@ $^ $(LDFLAGS_LINUX)

# Debug Windows target
$(BUILD_DIR_WIN_DEBUG)/$(TARGET_WIN_DEBUG): $(OBJS_WIN_DEBUG)
	$(CXX_WIN) $(CFLAGS_WIN_DEBUG) -o $@ $^ $(LDFLAGS_WIN)

# Object file compilation rules for Linux
$(BUILD_DIR_LINUX)/%.o: %.cpp
	$(CXX_LINUX) $(CFLAGS_LINUX) -c $< -o $@

$(BUILD_DIR_LINUX)/sudoku.o: sudoku.cpp sudoku.h
	$(CXX_LINUX) $(CFLAGS_LINUX) -c $< -o $@

$(BUILD_DIR_LINUX)/debug.o: debug.cpp sudoku.h
	$(CXX_LINUX) $(CFLAGS_LINUX) -c $< -o $@

$(BUILD_DIR_LINUX)/generatepuzzle.o: generatepuzzle.cpp generatepuzzle.h sudoku.h
	$(CXX_LINUX) $(CFLAGS_LINUX) -c $< -o $@

$(BUILD_DIR_LINUX)/sudoku_game.o: sudoku_game.cpp sudoku.h generatepuzzle.h
	$(CXX_LINUX) $(CFLAGS_LINUX) $(GTK_CFLAGS_LINUX) -c $< -o $@

# Object file compilation rules for Windows
$(BUILD_DIR_WIN)/%.win.o: %.cpp
	$(CXX_WIN) $(CFLAGS_WIN) -c $< -o $@

$(BUILD_DIR_WIN)/sudoku.win.o: sudoku.cpp sudoku.h
	$(CXX_WIN) $(CFLAGS_WIN) -c $< -o $@

$(BUILD_DIR_WIN)/debug.win.o: debug.cpp sudoku.h
	$(CXX_WIN) $(CFLAGS_WIN) -c $< -o $@

$(BUILD_DIR_WIN)/generatepuzzle.win.o: generatepuzzle.cpp generatepuzzle.h sudoku.h
	$(CXX_WIN) $(CFLAGS_WIN) -c $< -o $@

$(BUILD_DIR_WIN)/sudoku_game.win.o: sudoku_game.cpp sudoku.h generatepuzzle.h
	$(CXX_WIN) $(CFLAGS_WIN) $(GTK_CFLAGS_WIN) -c $< -o $@

# Object file compilation rules for Linux debug
$(BUILD_DIR_LINUX_DEBUG)/%.debug.o: %.cpp
	$(CXX_LINUX) $(CFLAGS_LINUX_DEBUG) -c $< -o $@

$(BUILD_DIR_LINUX_DEBUG)/sudoku.debug.o: sudoku.cpp sudoku.h
	$(CXX_LINUX) $(CFLAGS_LINUX_DEBUG) -c $< -o $@

$(BUILD_DIR_LINUX_DEBUG)/debug.debug.o: debug.cpp sudoku.h
	$(CXX_LINUX) $(CFLAGS_LINUX_DEBUG) -c $< -o $@

$(BUILD_DIR_LINUX_DEBUG)/generatepuzzle.debug.o: generatepuzzle.cpp generatepuzzle.h sudoku.h
	$(CXX_LINUX) $(CFLAGS_LINUX_DEBUG) -c $< -o $@

$(BUILD_DIR_LINUX_DEBUG)/sudoku_game.debug.o: sudoku_game.cpp sudoku.h generatepuzzle.h
	$(CXX_LINUX) $(CFLAGS_LINUX_DEBUG) $(GTK_CFLAGS_LINUX) -c $< -o $@

# Object file compilation rules for Windows debug
$(BUILD_DIR_WIN_DEBUG)/%.win.debug.o: %.cpp
	$(CXX_WIN) $(CFLAGS_WIN_DEBUG) -c $< -o $@

$(BUILD_DIR_WIN_DEBUG)/sudoku.win.debug.o: sudoku.cpp sudoku.h
	$(CXX_WIN) $(CFLAGS_WIN_DEBUG) -c $< -o $@

$(BUILD_DIR_WIN_DEBUG)/debug.win.debug.o: debug.cpp sudoku.h
	$(CXX_WIN) $(CFLAGS_WIN_DEBUG) -c $< -o $@

$(BUILD_DIR_WIN_DEBUG)/generatepuzzle.win.debug.o: generatepuzzle.cpp generatepuzzle.h sudoku.h
	$(CXX_WIN) $(CFLAGS_WIN_DEBUG) -c $< -o $@

$(BUILD_DIR_WIN_DEBUG)/sudoku_game.win.debug.o: sudoku_game.cpp sudoku.h generatepuzzle.h
	$(CXX_WIN) $(CFLAGS_WIN_DEBUG) $(GTK_CFLAGS_WIN) -c $< -o $@

# DLL collection for Windows builds
.PHONY: collect-dlls
collect-dlls: $(BUILD_DIR_WIN)/$(TARGET_WIN)
	@echo "Collecting DLLs for $(TARGET_WIN)..."
	@build/windows/collect_dlls.sh $(BUILD_DIR_WIN)/$(TARGET_WIN) $(DLL_SOURCE_DIR) $(BUILD_DIR_WIN)

.PHONY: collect-debug-dlls
collect-debug-dlls: $(BUILD_DIR_WIN_DEBUG)/$(TARGET_WIN_DEBUG)
	@echo "Collecting Debug DLLs for $(TARGET_WIN_DEBUG)..."
	@build/windows/collect_dlls.sh $(BUILD_DIR_WIN_DEBUG)/$(TARGET_WIN_DEBUG) $(DLL_SOURCE_DIR) $(BUILD_DIR_WIN_DEBUG)

# Python wrapper (if needed)
.PHONY: wrapper
wrapper: $(BUILD_DIR_LINUX)/sudoku_solver.so

$(BUILD_DIR_LINUX)/sudoku_solver.so: sudoku_wrapper.cpp sudoku.cpp debug.cpp generatepuzzle.cpp
	$(CXX_LINUX) $(CFLAGS_LINUX) $(shell python3-config --includes) -shared -fPIC -o $@ $^

# Clean target
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR_LINUX)/*
	rm -rf $(BUILD_DIR_WIN)/*
	rm -rf $(BUILD_DIR_LINUX_DEBUG)/*
	rm -rf $(BUILD_DIR_WIN_DEBUG)/*

# Install target (optional)
.PHONY: install
install: $(BUILD_DIR_LINUX)/$(TARGET_LINUX)
	install -m 755 $< /usr/local/bin/$(TARGET_LINUX)

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  make              - Build for both Linux and Windows (default)"
	@echo "  make linux        - Build for Linux only"
	@echo "  make windows      - Build for Windows only"
	@echo "  make linux-debug  - Build Linux debug version"
	@echo "  make windows-debug - Build Windows debug version"
	@echo "  make all-debug    - Build all debug versions"
	@echo "  make wrapper      - Build Python wrapper module"
	@echo "  make clean        - Remove all build files"
	@echo "  make install      - Install Linux binary to /usr/local/bin"
	@echo "  make help         - Show this help message"
