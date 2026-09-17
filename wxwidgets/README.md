# Sudoku Suite (wxWidgets, cross-platform)

Cross-platform (Linux + Windows) rewrite of the original Windows-only
C++/CLI + Windows Forms apps, using wxWidgets 3.2. Two executables share
one core solving/generation library:

- **sudoku_solver** — manual solver/workbench (toolbar-driven, all 10
  solving techniques exposed individually).
- **sudoku_game** — timed play mode with puzzle generation, progressive
  cell locking, sound feedback and a high-score table.

## Layout

Flat directory - everything builds from one folder:

```
sudoku.cpp/h, generatepuzzle.cpp/h, highscores.cpp/h  - unchanged
    solving/generation engine (already portable C++)
sudoku_wrapper.cpp/h  - port of the old C++/CLI SudokuWrapper
grid_widget.cpp/h     - shared 9x9 grid control (wxPanel), used by both apps
debugqueue.cpp        - portable replacement for winprint.cpp
game_sound.h           - cross-platform beep helper
solver_app.cpp         - port of win_main.cpp (Solver app)
game_app.cpp           - port of sudoku_game.cpp (Game app)
```

## Building

### Linux
```
sudo apt install libwxgtk3.2-dev cmake build-essential   # Ubuntu/Debian
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/sudoku_solver
./build/sudoku_game
```

### Windows
```
vcpkg install wxwidgets
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg>\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```
(The old `.vcxproj`/`.wxs`/`.rc` files are no longer used — CMake now
drives both platforms from the same source tree.)

Drop an `app.ico` next to the CMakeLists.txt before building and it will
be copied alongside both executables and loaded at startup.

## What changed vs. the original

The gameplay, puzzle generation, all 10 solving algorithms, save/load
format, undo history, and every keyboard shortcut are unchanged. A few
things had to change because they depended on Windows/.NET:

- **UI framework**: Windows Forms (C++/CLI, Windows-only) → wxWidgets
  (native on both platforms). Layout uses wxWidgets sizers instead of
  fixed pixel `Location`/`Size`, so windows are actually resizable
  without the manual `Form_Resize` recalculation the original needed.
- **Sound**: the original called Win32 `::Beep(freq, duration)` to play
  actual tones. There's no portable equivalent without an extra audio
  dependency, so non-Windows builds fall back to the system bell
  (`game_sound.h` explains the tradeoff and where to plug in a real
  audio library like SDL2_mixer if you want the original tunes on Linux).
- **Debug message queue**: the original used a Win32-only lock-free
  queue (`winprint.cpp`, built on `InterlockedIncrement`). Replaced with
  a portable `std::mutex`-protected queue (`debugqueue.cpp`) with the
  same producer/consumer contract.
- **Save file location**: the original solver wrote save files to the
  current directory; the game used `%APPDATA%`. Both now use
  `wxStandardPaths::GetUserDataDir()`, which resolves to the right
  per-user config directory on each platform.
- **Threaded solving**: the game app ran algorithm techniques on a
  background thread so the timer wouldn't stutter during solving. Since
  these techniques run in milliseconds on any real puzzle, both apps now
  run them synchronously — simpler, and imperceptible in practice.
- **Bug fix**: in the original solver, `ValidateAndHighlight()` reset
  *every* cell to plain white/black right before checking for conflicts,
  which wiped out the light-blue immutable-clue styling every time the
  grid refreshed. Fixed so immutable clues stay visibly blue.
- **Clipboard copy**: simplified from a full HTML clipboard format to
  plain text (the ASCII-art grid), since HTML clipboard formatting is a
  Windows-specific convention that most Linux apps don't consume anyway.

## Known limitations

- Win32 print support (`winprint.cpp`) was actually a debug-logging
  queue, not document printing — there was no real "print the puzzle"
  feature to port.
- Sound on Linux/macOS is a simple bell, not the original's tones (see
  above).
