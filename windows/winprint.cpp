#include "sudoku.h"
#include <iostream>
using namespace std;
#include <stdlib.h>

#include <vector>
#include <set>
#include <algorithm>

#include <fstream>
#include <ctime>
#include <cstring>

DebugQueue debugQueue;


void Sudoku::print_debug(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Add to queue
    if (debugQueue.size < DebugQueue::MAX_SIZE) {
        debugQueue.rear = (debugQueue.rear + 1) % DebugQueue::MAX_SIZE;
        strcpy_s(debugQueue.messages[debugQueue.rear], buffer);
        debugQueue.size++;
    }
}

char* Sudoku::get_next_debug_message() {
    if (debugQueue.size > 0) {
        char* msg = debugQueue.messages[debugQueue.front];
        debugQueue.front = (debugQueue.front + 1) % DebugQueue::MAX_SIZE;
        debugQueue.size--;
        return msg;
    }
    return nullptr;
}

