# Makefile for Sudoku - Multi-platform support (Linux and DOS)
# Supports both native Linux builds and DOS builds via DJGPP/Docker
VERSION=1.0

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM := linux
else
    PLATFORM := dos
endif

# Source files - using DOS-compatible versions
SRCS = sudoku.cpp generatepuzzle.cpp sudoku_main.cpp dosprint.cpp

# Output files
DOS_TARGET = sud.exe
DOS_COFF = sud
LINUX_TARGET = sudoku

# Build directories
ifeq ($(PLATFORM),linux)
    BUILD_DIR = build/linux
else
    BUILD_DIR = build/dos
endif
OBJ_DIR = $(BUILD_DIR)/obj

# Docker image for DJGPP (DOS only)
DJGPP_IMAGE = djfdyuruiry/djgpp

# Allegro paths (DOS only)
ALLEGRO_URL = https://github.com/superjamie/allegro-4.2.3.1-xc/archive/refs/heads/master.tar.gz
ALLEGRO_BUILD = $(BUILD_DIR)/source-install

# CSDPMI URL and paths (DOS only)
CSDPMI_URL = http://na.mirror.garr.it/mirrors/djgpp/current/v2misc/csdpmi7b.zip
CSDPMI_DIR = $(BUILD_DIR)/csdpmi

# Get current user and group IDs for Docker
USER_ID = $(shell id -u)
GROUP_ID = $(shell id -g)

# Compiler flags - C++11 compatible, no C++20 features
LINUX_CFLAGS = -O3 -fomit-frame-pointer -ffast-math -funroll-loops -fpermissive -w -std=c++11
DOS_CFLAGS = -O3 -march=i586 -fomit-frame-pointer -ffast-math -funroll-loops -fpermissive -w -std=c++11

# Default target - build for current platform
all: $(PLATFORM)

# Help target
help:
	@echo "Sudoku Makefile - Multi-platform Build System"
	@echo "=============================================="
	@echo "Detected platform: $(PLATFORM)"
	@echo ""
	@echo "Platform-specific targets:"
	@echo "  make linux        - Build native Linux executable"
	@echo "  make dos          - Build DOS executable (requires Docker and DJGPP)"
	@echo ""
	@echo "Linux build targets:"
	@echo "  make linux-build  - Compile for Linux"
	@echo "  make linux-run    - Build and run on Linux"
	@echo ""
	@echo "DOS build targets:"
	@echo "  make setup        - Setup DJGPP environment and CSDPMI (DOS)"
	@echo "  make allegro      - Build Allegro 4 library (DOS)"
	@echo "  make compile      - Quick compile with DJGPP (DOS)"
	@echo "  make run          - Run in DOSBox"
	@echo ""
	@echo "Common targets:"
	@echo "  make clean        - Clean all build files"
	@echo "  make clean-obj    - Clean only object files"
	@echo "  make status       - Show build status"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make              - Build for current platform ($(PLATFORM))"
	@echo "  make linux        - Force build for Linux"
	@echo "  make dos          - Force build for DOS"

# ============================================================================
# LINUX BUILD TARGETS
# ============================================================================

linux: linux-build
	@echo "✅ Linux build complete!"
	@echo "📁 Output: $(BUILD_DIR)/$(LINUX_TARGET)"

linux-build: $(BUILD_DIR)/$(LINUX_TARGET)

$(BUILD_DIR)/$(LINUX_TARGET): $(OBJ_DIR)/sudoku.o $(OBJ_DIR)/generatepuzzle.o $(OBJ_DIR)/linux_main.o
	@mkdir -p $(BUILD_DIR)
	@echo "Linking Linux executable..."
	g++ $^ -o $@ $(LINUX_CFLAGS) -lm
	@echo "✅ Linux executable created successfully"

$(OBJ_DIR)/sudoku.o: sudoku.cpp sudoku.h
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling sudoku.cpp for Linux..."
	g++ -c sudoku.cpp -o $@ $(LINUX_CFLAGS)

$(OBJ_DIR)/generatepuzzle.o: generatepuzzle.cpp generatepuzzle.h sudoku.h
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling generatepuzzle.cpp for Linux..."
	g++ -c generatepuzzle.cpp -o $@ $(LINUX_CFLAGS)

$(OBJ_DIR)/linux_main.o: sudoku_main.cpp sudoku.h generatepuzzle.h
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling sudoku_main.cpp for Linux..."
	g++ -c sudoku_main.cpp -o $@ $(LINUX_CFLAGS)

linux-run: linux
	@echo "Running Sudoku on Linux..."
	@./$(BUILD_DIR)/$(LINUX_TARGET)

# ============================================================================
# DOS BUILD TARGETS
# ============================================================================

# Setup DJGPP environment
setup: pull-djgpp get-csdpmi
	@echo "Setup complete"

# Pull DJGPP Docker image
pull-djgpp:
	@echo "Pulling DJGPP Docker image..."
	docker pull $(DJGPP_IMAGE)

# Download and extract CSDPMI
get-csdpmi:
	@if [ ! -d "$(CSDPMI_DIR)" ]; then \
		echo "Downloading CSDPMI..."; \
		mkdir -p $(BUILD_DIR); \
		cd $(BUILD_DIR) && \
		curl -L -o csdpmi7b.zip $(CSDPMI_URL) && \
		unzip -o -q csdpmi7b.zip -d csdpmi && \
		rm csdpmi7b.zip && \
		cd ../..; \
	else \
		echo "CSDPMI already downloaded"; \
	fi

# Build Allegro 4 library
allegro: pull-djgpp get-csdpmi
	@if [ ! -d "$(ALLEGRO_BUILD)" ]; then \
		echo "Building Allegro 4 for DJGPP..."; \
		mkdir -p $(BUILD_DIR); \
		cd $(BUILD_DIR) && \
		curl -L -o allegro-4.2.3.1-xc.tar.gz $(ALLEGRO_URL) && \
		docker run --rm \
			-v $(PWD)/$(BUILD_DIR):/workspace:z \
			-w /workspace \
			--user root \
			$(DJGPP_IMAGE) \
			/bin/bash -c " \
				tar xzf allegro-4.2.3.1-xc.tar.gz && \
				cd allegro-4.2.3.1-xc-master && \
				chmod +x xmake.sh && \
				./xmake.sh lib && \
				mkdir -p /workspace/source-install/include && \
				mkdir -p /workspace/source-install/lib && \
				cp -r include/* /workspace/source-install/include/ && \
				cp lib/djgpp/liballeg.a /workspace/source-install/lib/ && \
				chown -R $(USER_ID):$(GROUP_ID) /workspace && \
				echo 'Allegro 4 built successfully' \
			" && \
		cd ../..; \
	else \
		echo "Allegro 4 already built"; \
	fi

# Quick compile (assumes Allegro is already built)
# Uses DOS-compatible sudoku_main.cpp which doesn't require C++20
compile: allegro
	@echo "Compiling Sudoku with DJGPP and Allegro 4 (DOS-compatible)..."
	@mkdir -p $(OBJ_DIR)
	@docker run --rm \
		-v $(PWD):/src:z \
		-u $(USER_ID):$(GROUP_ID) \
		$(DJGPP_IMAGE) \
		/bin/sh -c " \
			cd /src && \
			echo 'Compiling sudoku.cpp...' && \
			g++ -c sudoku.cpp -I$(BUILD_DIR)/source-install/include $(DOS_CFLAGS) -o $(OBJ_DIR)/sudoku.o && \
			echo 'Compiling generatepuzzle.cpp...' && \
			g++ -c generatepuzzle.cpp -I$(BUILD_DIR)/source-install/include $(DOS_CFLAGS) -o $(OBJ_DIR)/generatepuzzle.o && \
			echo 'Compiling sudoku_main.cpp (DOS-compatible)...' && \
			g++ -c sudoku_main.cpp -I$(BUILD_DIR)/source-install/include $(DOS_CFLAGS) -o $(OBJ_DIR)/dos_main.o && \
			echo 'Linking executable...' && \
			g++ $(OBJ_DIR)/*.o -L$(BUILD_DIR)/source-install/lib -lalleg -lm $(DOS_CFLAGS) -s -o $(BUILD_DIR)/sudoku && \
			echo 'Converting to COFF format...' && \
			exe2coff $(BUILD_DIR)/sudoku && \
			echo 'Adding DPMI stub...' && \
			cat $(CSDPMI_DIR)/bin/CWSDSTUB.EXE $(BUILD_DIR)/sudoku > $(BUILD_DIR)/$(DOS_TARGET) && \
			echo 'DOS build complete!' && \
			rm $(BUILD_DIR)/sudoku \
		"
	@echo "✅ Build successful!"
	@echo "📁 Output: $(BUILD_DIR)/$(DOS_TARGET)"

# Full DOS build
dos: compile

# Run in DOSBox
run: dos
	@echo "Running Sudoku in DOSBox..."
	@if [ -f "$(BUILD_DIR)/$(DOS_TARGET)" ]; then \
		if command -v dosbox >/dev/null 2>&1; then \
			cd $(BUILD_DIR) && dosbox $(DOS_TARGET); \
		else \
			echo "DOSBox not found. Install with: sudo apt-get install dosbox"; \
			echo "Download from: https://www.dosbox.com/"; \
			exit 1; \
		fi \
	else \
		echo "ERROR: $(BUILD_DIR)/$(DOS_TARGET) not found. Build first with: make dos"; \
		exit 1; \
	fi

# Clean all build files
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@rm -f $(DOS_TARGET)
	@echo "✅ Clean complete"

# Clean only object files
clean-obj:
	@echo "Cleaning object files..."
	@rm -rf $(OBJ_DIR)
	@echo "✅ Object files cleaned"

# Clean Allegro build only
clean-allegro:
	@echo "Cleaning Allegro build..."
	@rm -rf $(ALLEGRO_BUILD)
	@echo "✅ Allegro cleaned"

# Status target - show build status
status:
	@echo "Build Status:"
	@echo "============"
	@echo "Current Platform: $(PLATFORM)"
	@echo ""
	@echo "Linux Build:"
	@if [ -f "build/linux/$(LINUX_TARGET)" ]; then \
		echo "✅ Executable: build/linux/$(LINUX_TARGET) ($(shell ls -lh build/linux/$(LINUX_TARGET) 2>/dev/null | awk '{print $$5}'))"; \
	else \
		echo "❌ Executable: Not built"; \
	fi
	@echo ""
	@echo "DOS Build:"
	@if [ -d "build/dos/csdpmi" ]; then \
		echo "✅ CSDPMI: Downloaded"; \
	else \
		echo "❌ CSDPMI: Not downloaded"; \
	fi
	@if [ -d "build/dos/source-install" ]; then \
		echo "✅ Allegro 4: Built"; \
	else \
		echo "❌ Allegro 4: Not built"; \
	fi
	@if [ -f "build/dos/$(DOS_TARGET)" ]; then \
		echo "✅ Executable: build/dos/$(DOS_TARGET) ($(shell ls -lh build/dos/$(DOS_TARGET) 2>/dev/null | awk '{print $$5}'))"; \
	else \
		echo "❌ Executable: Not built"; \
	fi

.PHONY: all help setup pull-djgpp get-csdpmi allegro compile dos linux linux-build linux-run run clean clean-obj clean-allegro status
