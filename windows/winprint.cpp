#include "sudoku.h"
#include <cstring>
#include <cstdio>

DebugQueue debugQueue;

void Sudoku::print_debug(const char* format, ...) {
    // Drop message immediately if queue is full - no blocking
    if (debugQueue.size >= DebugQueue::MAX_SIZE)
        return;

    char* slot = debugQueue.messages[(debugQueue.rear + 1) % DebugQueue::MAX_SIZE];

    va_list args;
    va_start(args, format);
    vsnprintf(slot, 256, format, args);
    va_end(args);

    debugQueue.rear = (debugQueue.rear + 1) % DebugQueue::MAX_SIZE;
    debugQueue.size++;
}

char* Sudoku::get_next_debug_message() {
    if (debugQueue.size == 0)
        return nullptr;
    char* msg = debugQueue.messages[debugQueue.front];
    debugQueue.front = (debugQueue.front + 1) % DebugQueue::MAX_SIZE;
    debugQueue.size--;
    return msg;
}
