#include "curses.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    initscr();
    
    // Print test line
    move(5, 5);
    printw("AAAAA BBBBB CCCCC DDDDD EEEEE");
    refresh();
    sleep(2);
    
    // Test clear from cursor to end
    move(5, 15);  // Move to middle of text
    printf("\033[s");  // Save cursor position
    printf("\033[0K"); // Should clear from cursor to end only
    printf("\033[u");  // Restore cursor position
    fflush(stdout);
    
    sleep(2);
    endwin();
    return 0;
}
