#include "sudoku.h"
#include <cstring>
#include <cstdio>
#include <windows.h>

// Lock-free single-producer single-consumer circular queue.
// Background solver thread writes (producer), UI timer thread reads (consumer).
// No mutex needed as long as only one thread writes and one thread reads.
DebugQueue debugQueue;

void Sudoku::print_debug(const char* format, ...) {
    // Read size with acquire fence so we see the latest consumer state
    int sz = InterlockedCompareExchange((volatile LONG*)&debugQueue.size, 0, 0);
    if (sz >= DebugQueue::MAX_SIZE)
        return;  // Full - drop and return immediately, no blocking

    int slot = (debugQueue.rear + 1) % DebugQueue::MAX_SIZE;

    va_list args;
    va_start(args, format);
    // _vsnprintf_l with NULL locale avoids per-call locale lookup overhead
    vsnprintf(debugQueue.messages[slot], 255, format, args);
    va_end(args);
    debugQueue.messages[slot][255] = '\0';

    // Publish: update rear then increment size with a full memory barrier
    // so the consumer never sees size incremented before the message is written
    debugQueue.rear = slot;
    InterlockedIncrement((volatile LONG*)&debugQueue.size);
}

char* Sudoku::get_next_debug_message() {
    // Read size with acquire semantics
    if (InterlockedCompareExchange((volatile LONG*)&debugQueue.size, 0, 0) == 0)
        return nullptr;

    char* msg = debugQueue.messages[debugQueue.front];
    debugQueue.front = (debugQueue.front + 1) % DebugQueue::MAX_SIZE;
    // Decrement size with release semantics so producer sees the free slot
    InterlockedDecrement((volatile LONG*)&debugQueue.size);
    return msg;
}
