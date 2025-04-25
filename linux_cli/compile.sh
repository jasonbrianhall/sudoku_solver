#!/bin/bash

# Variables
DJGPP_IMAGE="djfdyuruiry/djgpp"
PDCURSES_REPO="https://github.com/wmcbrine/PDCurses.git"
PDCURSES_DIR="pdcurses_build"
CSDPMI_URL="http://na.mirror.garr.it/mirrors/djgpp/current/v2misc/csdpmi7b.zip"
USER_ID=$(id -u)
GROUP_ID=$(id -g)
DOS_TARGET="sudoku.exe"
COFF_TARGET="sudoku"

# Check and compile Linux version if g++ is available
if command -v g++ &> /dev/null; then
    echo "Compiling Linux version..."
    # Resolve symlinks and compile with all source files
    g++ main.cpp $(readlink -f sudoku.cpp) $(readlink -f generatepuzzle.cpp) unixprint.cpp -lncurses -o sudoku
    if [ $? -eq 0 ]; then
        echo "Linux build successful: ./sudoku"
    else
        echo "Linux build failed"
    fi
else
    echo "g++ not found - skipping Linux build"
fi

# Check and compile Windows version if cross-compiler is available
if command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    mkdir -p win_sudoku

    echo "Compiling Windows version (PDCurses) ..."
    x86_64-w64-mingw32-g++ main.cpp $(readlink -f sudoku.cpp) $(readlink -f generatepuzzle.cpp) unixprint.cpp -lpdcurses -std=c++14 -o win_sudoku/sudoku.exe
    
    if [ $? -eq 0 ]; then
        # Collect DLLs if we have a helper script
        if [ -f "./collect_dlls.sh" ]; then
            ./collect_dlls.sh win_sudoku/sudoku.exe /usr/x86_64-w64-mingw32/sys-root/mingw/bin win_sudoku
        else
            echo "DLL collection script not found. You may need to manually copy required DLLs."
        fi
        echo "Windows build successful: win_sudoku/sudoku.exe"
    else
        echo "Windows build failed"
    fi
else
    echo "Windows cross-compiler not found - skipping Windows build"
fi

# Build MSDOS version with PDCurses using Docker
echo "Building MSDOS version with PDCurses..."

# Pull the DJGPP Docker image
echo "Pulling DJGPP Docker image..."
docker pull ${DJGPP_IMAGE}

# Download CSDPMI if needed
if [ ! -d "csdpmi" ]; then
    echo "Downloading CSDPMI..."
    wget ${CSDPMI_URL}
    mkdir -p csdpmi
    unzip -o csdpmi7b.zip -d csdpmi
fi

# Clone PDCurses if needed
if [ ! -d "${PDCURSES_DIR}" ]; then
    echo "Cloning PDCurses repository..."
    git clone ${PDCURSES_REPO} ${PDCURSES_DIR}
fi

# Create a script to run inside Docker for building PDCurses and the application
cat > build_msdos.sh << 'EOF'
#!/bin/bash
set -e

# Build PDCurses for MSDOS
cd /src/pdcurses_build/dos
echo "Building PDCurses for MSDOS..."
# First clean any existing build artifacts to ensure a fresh build
make -f Makefile clean PDCURSES_SRCDIR=/src/pdcurses_build PLATFORM=djgpp
# Now build PDCurses
make -f Makefile PDCURSES_SRCDIR=/src/pdcurses_build PLATFORM=djgpp

# Move PDCurses library to a location we can reference
mkdir -p /src/lib /src/include
cp pdcurses.a /src/lib/libpdcurses.a
cp /src/pdcurses_build/curses.h /src/include/
cp /src/pdcurses_build/panel.h /src/include/

# Create an empty term.h if needed by your code
touch /src/include/term.h

# Build the sudoku application
cd /src
echo "Building sudoku for MSDOS..."
# Add preprocessor define for MSDOS to handle any platform-specific code
g++ -s main.cpp sudoku.cpp generatepuzzle.cpp unixprint.cpp -o sudoku.exe -I/src/include -L/src/lib -lpdcurses -DMSDOS

echo "Build complete!"
EOF

# Make the script executable
chmod +x build_msdos.sh

# Create a temporary directory to avoid symlink issues
TEMP_BUILD_DIR=$(mktemp -d)
echo "Created temporary build directory: ${TEMP_BUILD_DIR}"

# Resolve symbolic links for source files
echo "Resolving symbolic links for source files..."

# Copy main.cpp directly
cp -f main.cpp "${TEMP_BUILD_DIR}/"

# For sudoku.cpp - follow the symlink to get the actual file
SUDOKU_CPP_REAL=$(readlink -f sudoku.cpp)
if [ -f "$SUDOKU_CPP_REAL" ]; then
    echo "Found real sudoku.cpp at: $SUDOKU_CPP_REAL"
    cp "$SUDOKU_CPP_REAL" "${TEMP_BUILD_DIR}/sudoku.cpp"
else
    echo "Error: Could not resolve symlink for sudoku.cpp"
    exit 1
fi

# For sudoku.h - follow the symlink to get the actual file
SUDOKU_H_REAL=$(readlink -f sudoku.h)
if [ -f "$SUDOKU_H_REAL" ]; then
    echo "Found real sudoku.h at: $SUDOKU_H_REAL"
    cp "$SUDOKU_H_REAL" "${TEMP_BUILD_DIR}/sudoku.h"
else
    echo "Error: Could not resolve symlink for sudoku.h"
    exit 1
fi

# For generatepuzzle.cpp - follow the symlink to get the actual file
GENERATE_CPP_REAL=$(readlink -f generatepuzzle.cpp)
if [ -f "$GENERATE_CPP_REAL" ]; then
    echo "Found real generatepuzzle.cpp at: $GENERATE_CPP_REAL"
    cp "$GENERATE_CPP_REAL" "${TEMP_BUILD_DIR}/generatepuzzle.cpp"
else
    echo "Error: Could not resolve symlink for generatepuzzle.cpp"
    exit 1
fi

# For generatepuzzle.h - follow the symlink to get the actual file
GENERATE_H_REAL=$(readlink -f generatepuzzle.h)
if [ -f "$GENERATE_H_REAL" ]; then
    echo "Found real generatepuzzle.h at: $GENERATE_H_REAL"
    cp "$GENERATE_H_REAL" "${TEMP_BUILD_DIR}/generatepuzzle.h"
else
    echo "Error: Could not resolve symlink for generatepuzzle.h"
    exit 1
fi

# Copy unixprint.cpp for reference but create a DOS-specific version
cp -f unixprint.cpp "${TEMP_BUILD_DIR}/"

# Copy PDCurses and build script
cp -r "${PDCURSES_DIR}" "${TEMP_BUILD_DIR}/"
cp build_msdos.sh "${TEMP_BUILD_DIR}/"

# List files in the temporary directory for verification
echo "Files in temporary build directory:"
ls -la "${TEMP_BUILD_DIR}"

# Run the Docker container with the temporary directory
echo "Starting Docker build process..."
docker run --rm -v "${TEMP_BUILD_DIR}:/src:z" -u ${USER_ID}:${GROUP_ID} ${DJGPP_IMAGE} /src/build_msdos.sh

# Copy back the built files
echo "Copying built files from temporary directory..."
cp "${TEMP_BUILD_DIR}/${DOS_TARGET}" ./ 2>/dev/null || echo "Failed to copy executable"
cp "${TEMP_BUILD_DIR}/lib/libpdcurses.a" ./ 2>/dev/null || echo "Failed to copy PDCurses library"

# Clean up
echo "Cleaning up temporary directory..."
rm -rf "${TEMP_BUILD_DIR}"

if [ -f "csdpmi/bin/CWSSTUB.EXE" ]; then
    docker run --rm -v "${TEMP_BUILD_DIR}:/src:z" -u ${USER_ID}:${GROUP_ID} ${DJGPP_IMAGE} "exe2coff $(DOS_TARGET) && \
    cat csdpmi/bin/CWSDSTUB.EXE $(DOS_COFF) $(COFF_TARGET) > $(DOS_TARGET)"
fi

# Check if build was successful
if [ -f "${DOS_TARGET}" ]; then
    echo "MSDOS build successful! Files created:"
    echo "- ${DOS_TARGET}"
    echo "To run in DOSBox, execute: dosbox ${DOS_TARGET}"
else
    echo "MSDOS build failed."
fi
