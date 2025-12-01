#include <ncurses.h> 
#include <stdlib.h> 
#include <unistd.h> // for usleep()
#include <signal.h>
#include <time.h> 
#include <string.h>

// INCOMPLETE CODE BELOW **WILL NOT COMPILE**:
// Added rotations according to SRS standard
// https://tetris.wiki/Super_Rotation_System

// TODO: Test rotations and wall kicks, implement gravity and locking, line clears, scoring, level progression, and rendering

// struct Block {
//     char structure[4][2];
// };


// struct Block cube = {.structure = {{'O','O'},
//                                    {'O','O'},
//                                    {'x','x'},
//                                    {'x','x'}}};

// struct Block line = {.structure = {{'O','x'},
//                                    {'O','x'},
//                                    {'O','x'},
//                                    {'O','x'}}};

// struct Block reverseL = {.structure = {{'O','O'},
//                                        {'O','x'},
//                                        {'O','x'},
//                                        {'x','x'}}};

// struct Block regL = {.structure = {{'O','x'},
//                                    {'O','x'},
//                                    {'O','O'},
//                                    {'x','x'}}};

// struct Block tee = {.structure = {{'O','x'},
//                                   {'O','O'},
//                                   {'O','x'},
//                                   {'x','x'}}};

// struct Block regZ = {.structure = {{'x','O'},
//                                    {'O','O'},
//                                    {'O','x'},
//                                    {'x','x'}}};

// struct Block reverseZ = {.structure = {{'O','x'},
//                                        {'O','O'},
//                                        {'x','O'},
//                                        {'x','x'}}};

static int wallKickNonI[8][5][2] = { // SRS Standard Wall Kick Data for non-I pieces
    {{0,0},{-1,0},{-1,1},{0,-2},{-1,-2}},
    {{0,0},{1,0},{1,-1},{0,2},{1,2}},
    {{0,0},{1,0},{1,-1},{0,2},{1,2}},
    {{0,0},{-1,0},{-1,1},{0,-2},{-1,-2}},
    {{0,0},{1,0},{1,1},{0,-2},{1,-2}},
    {{0,0},{-1,0},{-1,-1},{0,2},{-1,2}},
    {{0,0},{-1,0},{-1,-1},{0,2},{-1,2}},
    {{0,0},{1,0},{1,1},{0,-2},{1,-2}},
};

static int wallKickI[8][5][2] = { // SRS Standard Wall Kick Data for I pieces
    {{0,0},{-2,0},{1,0},{-2,-1},{1,2}},
    {{0,0},{2,0},{-1,0},{+2,+1},{-1,-2}},
    {{0,0},{-1,0},{2,0},{-1,2},{2,-1}},
    {{0,0},{1,0},{-2,0},{1,-2},{-2,1}},
    {{0,0},{2,0},{-1,0},{2,1},{-1,-2}},
    {{0,0},{-2,0},{1,0},{-2,-1},{1,2}},
    {{0,0},{1,0},{-2,0},{1,-2},{-2,1}},
    {{0,0},{-1,0},{2,0},{-1,2},{2,-1}},
};

struct Template { // Define the block templates with their orientations (4 sets of 4x4 matrices, one for each rotation)
    int orientations[4][4][4]; //Anchored at top-left corner
    //0 for empty, 1-7 for occupied (different numbers for different pieces for easier rendering)
    int startingColumn; // Starting column for spawn position (different depending on rotation center)
    int *wallKickData[8][5][2]; // Wall kick data for this piece
};

struct Block {
    struct Template *template; // Pointer to the block's template
    int orientation; // 0-3 for the four rotation states
    int position[2]; // row and column on the board
};

static struct Template i = {.orientations = {{{0, 0, 0, 0},{1, 1, 1, 1},{0, 0, 0, 0},{0, 0, 0, 0}},
    {{0, 0, 1, 0},{0, 0, 1, 0},{0, 0, 1, 0},{0, 0, 1, 0}},
    {{0, 0, 0, 0},{0, 0, 0, 0},{1, 1, 1, 1},{0, 0, 0, 0}},
    {{0, 1, 0, 0},{0, 1, 0, 0},{0, 1, 0, 0},{0, 1, 0, 0}}},
.startingColumn = 3,
.wallKickData = &wallKickI,
};

static struct Template j = {.orientations = {{{2, 0, 0, 0},{2, 2, 2, 0},{0, 0, 0, 0},{0, 0, 0, 0}},
    {{0, 2, 2, 0},{0, 2, 0, 0},{0, 2, 0, 0},{0, 0, 0, 0}},
    {{0, 0, 0, 0},{2, 2, 2, 0},{0, 0, 2, 0},{0, 0, 0, 0}},
    {{0, 2, 0, 0},{0, 2, 0, 0},{2, 2, 0, 0},{0, 0, 0, 0}}},
.startingColumn = 3,
.wallKickData = &wallKickNonI,
};

static struct Template l = {.orientations = {{{0, 0, 3, 0},{3, 3, 3, 0},{0, 0, 0, 0},{0, 0, 0, 0}},
    {{0, 3, 0, 0},{0, 3, 0, 0},{0, 3, 3, 0},{0, 0, 0, 0}},
    {{0, 0, 0, 0},{3, 3, 3, 0},{3, 0, 0, 0},{0, 0, 0, 0}},
    {{3, 3, 0, 0},{0, 3, 0, 0},{0, 3, 0, 0},{0, 0, 0, 0}}},
.startingColumn = 3,
.wallKickData = &wallKickNonI,
};

static struct Template o = {.orientations = {{{0, 4, 4, 0},{0, 4, 4, 0},{0, 0, 0, 0},{0, 0, 0, 0}},
    {{0, 4, 4, 0},{0, 4, 4, 0},{0, 0, 0, 0},{0, 0, 0, 0}},
    {{0, 4, 4, 0},{0, 4, 4, 0},{0, 0, 0, 0},{0, 0, 0, 0}},
    {{0, 4, 4, 0},{0, 4, 4, 0},{0, 0, 0, 0},{0, 0, 0, 0}}},
.startingColumn = 4,
.wallKickData = &wallKickNonI,
};

static struct Template s = {.orientations = {{{0, 5, 5, 0},{5, 5, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},
    {{0, 5, 0, 0},{0, 5, 5, 0},{0, 0, 5, 0},{0, 0, 0, 0}},
    {{0, 0, 0, 0},{0, 5, 5, 0},{5, 5, 0, 0},{0, 0, 0, 0}},
    {{5, 0, 0, 0},{5, 5, 0, 0},{5, 0, 0, 0},{0, 0, 0, 0}}},
.startingColumn = 3,
.wallKickData = &wallKickNonI,
};

static struct Template t = {.orientations = {{{0, 6, 0, 0},{6, 6, 6, 0},{0, 0, 0, 0},{0, 0, 0, 0}},
    {{0, 6, 0, 0},{0, 6, 6, 0},{0, 6, 0, 0},{0, 0, 0, 0}},
    {{0, 0, 0, 0},{6, 6, 6, 0},{0, 6, 0, 0},{0, 0, 0, 0}},
    {{0, 6, 0, 0},{6, 6, 0, 0},{0, 6, 0, 0},{0, 0, 0, 0}}},
.startingColumn = 3,
.wallKickData = &wallKickNonI,
};

static struct Template z = {.orientations = {{{7, 7, 0, 0},{0, 7, 7, 0},{0, 0, 0, 0},{0, 0, 0, 0}},
    {{0, 0, 7, 0},{0, 7, 7, 0},{0, 7, 0, 0},{0, 0, 0, 0}},
    {{0, 0, 0, 0},{7, 7, 0, 0},{0, 7, 7, 0},{0, 0, 0, 0}},
    {{0, 7, 0, 0},{7, 7, 0, 0},{7, 0, 0, 0},{0, 0, 0, 0}}},
.startingColumn = 3,
.wallKickData = &wallKickNonI,
};

struct Template templates[7]; // Array to hold all block templates
int board[22][10] = {0}; // Extra 2 rows for spawn area
int displayBoard[20][10] = {0}; // What the player actually sees (board + active piece)
int bag[7] = {0,0,0,0,0,0,0}; // 7-bag randomizer
int bagIndex = 0;

struct Block current; // Current active block

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
    int numb = rand() % 7;
    refresh();
    sleep(3);
    endwin();
    exit(0);
}

void initialize() {
    templates[0] = o;
    templates[1] = i;
    templates[2] = j;
    templates[3] = l;
    templates[4] = s;
    templates[5] = t;
    templates[6] = z;

    initscr(); // Start ncurses mode
    clear();
    noecho(); // Do not show key presses
    curs_set(0); // Hide the blinking cursor
    keypad(stdscr, TRUE); // Enable arrow keys
    nodelay(stdscr, TRUE); // Do not wait for key press
}

// void rotate() {
//     if (angle != 3) {
//         angle++;
//     }
//     else {
//         angle = 0;
//     }
// }

void rotate(int dir) { //1 for clockwise, -1 for counterclockwise
    //SRS Standard: If rotation causes collision, translate piece according to wall kick data
    //Ex: Rotate 0 to R: Try rotating translated at (0,0). If collision, try (-1,0). If collision, try (-1,+1), etc.
    //https://tetris.wiki/Super_Rotation_System

    int potentialOrientation = (current.orientation + dir) % 4;
    if (potentialOrientation < 0) potentialOrientation += 4;

    int wallKickIndex = current.orientation * 2 + (dir == 1 ? 0 : 1);
    for (int i = 0; i < 5; i++) {
        int dx = current.template->wallKickData[wallKickIndex][i][0];
        int dy = current.template->wallKickData[wallKickIndex][i][1];

        // Check if the piece can be placed at the new position
        int invalid = 0;
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (current.template->orientations[potentialOrientation][r][c] != 0) {
                    int boardRow = dx + r + current.position[0];
                    int boardCol = dy + c + current.position[1];
                    if (boardRow < 0 || boardRow >= 22 || boardCol < 0 || boardCol >= 10 || board[boardRow][boardCol] != 0) {
                        invalid = 1;
                        break;
                    }
                }
            }
            if (invalid) break;
        }

        if (!invalid) {
            current.orientation = potentialOrientation;
            current.position[0] += dy;
            current.position[1] += dx;
            return;
        }
    }
}

void randomize_bag() { //Tetris uses a "7-bag" randomizer to ensure uniform distribution of pieces
    bagIndex = 0;
    int count = 0;
    while (count < 7) {
        int r = rand() % 7;
        if (bag[r] == 0) {
            bag[r] = count++;
        }
    }
}

void spawn() {
    if (bagIndex == 7) randomize_bag();
    current.template = &templates[bag[bagIndex++]];

    current.orientation = 0;
    current.position[0] = 0; // row
    current.position[1] = current.template->startingColumn; // column
}

// void placeblock() {
//     for (int i = 0; i < 4; i++) {
//         for (int j = 0; j < 2; j++) {
//             if (current.structure[i][j] != 'x') {
//                 if (angle == 0) {
//                     mvprintw(fall+i, LR+j, "%c", current.structure[i][j]);
//                 }
//                 else if (angle == 1) {
//                     mvprintw(fall+j, LR-i, "%c", current.structure[i][j]);
//                 }
//                 else if (angle == 2) {
//                     mvprintw(fall-i, LR-j, "%c", current.structure[i][j]);
//                 }
//                 else if (angle == 3) {
//                     mvprintw(fall-j, LR+i, "%c", current.structure[i][j]);
//                 }
//             }
//         }
//     }
// }

// void assignblock () {
//     for (int k = 0; k < 4; k++) {
//         if (numb == 0) {
//             strcpy(current.structure[k], cube.structure[k]);
//         }
//         else if (numb == 1) {
//             strcpy(current.structure[k], line.structure[k]);
//         }
//         else if (numb == 2) {
//             strcpy(current.structure[k], reverseL.structure[k]);
//         }
//         else if (numb == 3) {
//             strcpy(current.structure[k], regL.structure[k]);
//         }
//         else if (numb == 4) {
//             strcpy(current.structure[k], tee.structure[k]);
//         }
//         else if (numb == 5) {
//             strcpy(current.structure[k], regZ.structure[k]);
//         }
//         else if (numb == 6) {
//             strcpy(current.structure[k], reverseZ.structure[k]);
//         }
//     }
// }

void handle_sigint(int sig) { // Print a message and exit cleanly
    endwin(); // End ncurses session
    printf("Game terminated\n");
    exit(0); // Exit the program
}

void change_direction(int ch) {
    if (ch == KEY_RIGHT) {
        rotate(1);
    }
    else if (ch == KEY_LEFT) {
        rotate(-1);
    }
}
