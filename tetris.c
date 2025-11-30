#include <ncurses.h> 
#include <stdlib.h> 
#include <unistd.h> // for usleep()
#include <signal.h>
#include <time.h> 
#include <string.h>

struct Block {
    char structure[4][2];
};


struct Block cube = {.structure = {{'O','O'},
                                   {'O','O'},
                                   {'x','x'},
                                   {'x','x'}}};

struct Block line = {.structure = {{'O','x'},
                                   {'O','x'},
                                   {'O','x'},
                                   {'O','x'}}};

struct Block reverseL = {.structure = {{'O','O'},
                                       {'O','x'},
                                       {'O','x'},
                                       {'x','x'}}};

struct Block regL = {.structure = {{'O','x'},
                                   {'O','x'},
                                   {'O','O'},
                                   {'x','x'}}};

struct Block tee = {.structure = {{'O','x'},
                                  {'O','O'},
                                  {'O','x'},
                                  {'x','x'}}};

struct Block regZ = {.structure = {{'x','O'},
                                   {'O','O'},
                                   {'O','x'},
                                   {'x','x'}}};

struct Block reverseZ = {.structure = {{'O','x'},
                                       {'O','O'},
                                       {'x','O'},
                                       {'x','x'}}};

struct Block current;
int angle = 0;
int fall = 0;
int LR = 5;
int numb;

void initialize();
void handle_sigint(int sig);
void rotate();
void placeblock();
void assignblock();
void change_direction(int ch);

int main() {
    signal(SIGINT, handle_sigint);
    initialize();
    refresh();
    sleep(3);
    endwin();
    exit(0);
}

void initialize() {
    initscr(); // Start ncurses mode
    clear();
    noecho(); // Do not show key presses
    curs_set(0); // Hide the blinking cursor
    keypad(stdscr, TRUE); // Enable arrow keys
    nodelay(stdscr, TRUE); // Do not wait for key press
}

void rotate() {
    if (angle != 3) {
        angle++;
    }
    else {
        angle = 0;
    }
}

void placeblock() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            if (current.structure[i][j] != 'x') {
                if (angle == 0) {
                    mvprintw(fall+i, LR+j, "%c", current.structure[i][j]);
                }
                else if (angle == 1) {
                    mvprintw(fall+j, LR-i, "%c", current.structure[i][j]);
                }
                else if (angle == 2) {
                    mvprintw(fall-i, LR-j, "%c", current.structure[i][j]);
                }
                else if (angle == 3) {
                    mvprintw(fall-j, LR+i, "%c", current.structure[i][j]);
                }
            }
        }
    }
}

void assignblock () {
    for (int k = 0; k < 4; k++) {
        strcpy(current.structure[k], regL.structure[k]);
    }
}

void handle_sigint(int sig) { // Print a message and exit cleanly
    endwin(); // End ncurses session
    printf("Game terminated\n");
    exit(0); // Exit the program
}

void change_direction(int ch) {
    if (ch == KEY_RIGHT) {
        rotate();
    }
}
