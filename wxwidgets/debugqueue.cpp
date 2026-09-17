// Portable replacement for winprint.cpp.
//
// The original winprint.cpp implemented Sudoku::print_debug() and
// get_next_debug_message() using a lock-free circular buffer built on
// Win32 Interlocked* intrinsics, because it only ever needed to run on
// Windows. That made it Windows-only.
//
// This version keeps the same producer/consumer contract (solver thread
// writes, UI thread drains on a timer) but uses std::mutex + std::deque
// so it compiles and works identically on Windows, Linux and macOS.

#include "sudoku.h"
#include <cstdio>
#include <cstdarg>

void Sudoku::print_debug(const char* format, ...) {
    char buffer[256];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = '\0';

    std::lock_guard<std::mutex> lock(debugQueue.mtx);
    if (debugQueue.messages.size() >= 1024) {
        // Full - drop oldest rather than blocking the solver thread.
        debugQueue.messages.pop_front();
    }
    debugQueue.messages.emplace_back(buffer);
}

std::string Sudoku::get_next_debug_message() {
    std::lock_guard<std::mutex> lock(debugQueue.mtx);
    if (debugQueue.messages.empty()) {
        return std::string();
    }
    std::string msg = std::move(debugQueue.messages.front());
    debugQueue.messages.pop_front();
    return msg;
}
