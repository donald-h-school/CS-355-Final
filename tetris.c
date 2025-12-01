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
    int (*wallKickData)[8][5][2]; // Wall kick data for this piece
};

struct Block {
    struct Template *template; // Pointer to the block's template
    int orientation; // 0-3 for the four rotation states
    int position[2]; // row and column on the board
    int isColliding; // Flag to indicate if the block is colliding
    float gravityCounter; // Counter for gravity application
    int contactCounter; // Counter for contact with the ground
    int lockDelayResets; // Number of times lock delay has been reset
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
int bag[7] = {-1,-1,-1,-1,-1,-1,-1}; // 7-bag randomizer
int bagIndex = 0;

struct Block current = {
    .template = NULL,
    .orientation = 0,
    .position = {0, 0},
}; // Current active block

// int angle = 0;
// int fall = 0;
// int LR = 5;
// int numb;

int frameLength = 16667; // in microseconds
int level = 0;
int softDrop = 0;

void initialize();
void handle_sigint(int sig);
void rotate(int dir);
void translate(int dir);
void placeblock();
void assignblock();
void ctrls(int ch);
void tick();
void render();
void spawn();
void randomize_bag();
double getGravity();
void checkCollisions();
void lock();

int main() {
    signal(SIGINT, handle_sigint);
    initialize();
    //spawn();
    
    int ch;

    while (1) { // Press ctrl+c to exit
        ch = getch();
        if (ch == 'q') break;
        else if (ch != ERR) {
            ctrls(ch);
        }
        tick();
        refresh();
        render();
        usleep(frameLength);
    }
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

    randomize_bag();

    initscr(); // Start ncurses mode
    clear();
    noecho(); // Do not show key presses
    curs_set(0); // Hide the blinking cursor
    keypad(stdscr, TRUE); // Enable arrow keys
    nodelay(stdscr, TRUE); // Do not wait for key press
}

void tick() {
    if (current.template == NULL) {
        spawn();
    }

    if (current.isColliding) {
        current.contactCounter++;
        if (current.contactCounter >= 30) {
            lock();
        }
    }

    current.gravityCounter += getGravity();
    while(current.gravityCounter >= 1) {
        current.gravityCounter -= 1;
        checkCollisions();
        if (!current.isColliding) {
            current.position[0] += 1;
        }
    }
}

void checkCollisions() {
    int colliding = 0;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (current.template->orientations[current.orientation][r][c] != 0) {
                int boardRow = current.position[0] + r;
                int boardCol = current.position[1] + c;
                if (boardRow >= 21 || (boardRow >= 0 && board[boardRow + 1][boardCol] != 0)) {
                    colliding = 1;
                    softDrop = 0;
                    break;
                }
            }
        }
        if (colliding) break;
    }
    if (colliding && !current.isColliding) {
        current.isColliding = 1;
        current.lockDelayResets = 0;
        current.contactCounter = 0;
    } else if (!colliding) {
        current.isColliding = 0;
    }
}

void lock() {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (current.template->orientations[current.orientation][r][c] != 0) {
                int boardRow = current.position[0] + r;
                int boardCol = current.position[1] + c;
                if (boardRow >= 0 && boardRow < 22 && boardCol >= 0 && boardCol < 10) {
                    board[boardRow][boardCol] = current.template->orientations[current.orientation][r][c];
                }
            }
        }
    }
    // Reset current piece
    current.template = NULL;
    current.isColliding = 0;
    current.contactCounter = 0;
}

// void rotate() {
//     if (angle != 3) {
//         angle++;
//     }
//     else {
//         angle = 0;
//     }
// }

void translate(int dir) { //-1 for left, 1 for right
    if (current.template == NULL) return; // No piece to translate

    int potentialTranslation = current.position[1] + dir;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (current.template->orientations[current.orientation][r][c] != 0) {
                int boardRow = current.position[0] + r;
                int boardCol = potentialTranslation + c;
                if (boardCol < 0 || boardCol >= 10 || (boardRow >= 0 && board[boardRow][boardCol] != 0)) {
                    return; // Collision detected, cannot translate
                }
            }
        }
    }

    current.position[1] = potentialTranslation;
    checkCollisions();
    if (current.isColliding) {
        current.lockDelayResets++;
        current.contactCounter = 0;
    }
}

void rotate(int dir) { //1 for clockwise, -1 for counterclockwise
    //SRS Standard: If rotation causes collision, translate piece according to wall kick data
    //Ex: Rotate 0 to R: Try rotating translated at (0,0). If collision, try (-1,0). If collision, try (-1,+1), etc.
    //https://tetris.wiki/Super_Rotation_System

    if (current.template == NULL) return; // No piece to rotate

    int potentialOrientation = (current.orientation + dir) % 4;
    if (potentialOrientation < 0) potentialOrientation += 4;

    int wallKickIndex = (2 * current.orientation + (dir == 1 ? 0 : 7)) % 8;
    for (int i = 0; i < 5; i++) {
        int dx = (*current.template->wallKickData)[wallKickIndex][i][0];
        int dy = (*current.template->wallKickData)[wallKickIndex][i][1];

        // Check if the piece can be placed at the new position
        int invalid = 0;
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (current.template->orientations[potentialOrientation][r][c] != 0) {
                    int boardRow = dy + r + current.position[0];
                    int boardCol = dx + c + current.position[1];
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

            checkCollisions();
            if (current.isColliding) {
                current.lockDelayResets++;
                current.contactCounter = 0;
            }
        }
    }
}

void randomize_bag() { //Tetris uses a "7-bag" randomizer to ensure uniform distribution of pieces
    for (int i = 0; i < 7; i++) {
        bag[i] = -1;
    }
    bagIndex = 0;
    int count = 0;
    while (count < 7) {
        int r = rand() % 7;
        if (bag[r] == -1) {
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
    current.isColliding = 0;
    current.gravityCounter = 0;
    current.contactCounter = 0;
    current.lockDelayResets = 0;
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

void ctrls(int ch) {
    if (current.lockDelayResets >= 15) return; // No inputs allowed after 15 lock delay resets
    else if (ch == 'x') {
        rotate(1);
    }
    else if (ch == 'z') {
        rotate(-1);
    }
    else if (ch == KEY_DOWN) {
        softDrop = 1;
    }
    else if (ch == KEY_LEFT) {
        translate(-1);
    }
    else if (ch == KEY_RIGHT) {
        translate(1);
    }
}

void render() {
    //Todo: add advanced rendering logic
    for (int r = 2; r < 22; r++) {
        for (int c = 0; c < 10; c++) {
            if (board[r][c] != 0) {
                mvprintw(r, c*2, "[]");
            } else {
                mvprintw(r, c*2, "  ");
            }
        }
    }

    if (current.template != NULL) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (current.template->orientations[current.orientation][r][c] != 0) {
                    int boardRow = current.position[0] + r;
                    int boardCol = current.position[1] + c;
                    if (boardRow >= 2 && boardRow < 22 && boardCol >= 0 && boardCol < 10) {
                        if (board[boardRow][boardCol] != 0) mvprintw(boardRow, boardCol*2, "XX"); // Indicate collision
                        else mvprintw(boardRow, boardCol*2, "<>");
                    }
                }
            }
        }
    }
}

double gravity[25] = {
    0.01667, 0.021017, 0.026977, 0.035256, 0.04693,
    0.06361, 0.0879, 0.1236, 0.1774, 0.2598,
    0.388, 0.388, 0.388, 0.59, 0.59,
    0.59, 0.92, 0.92, 0.92, 1.46, 
    2.36, 3.91, 6.14, 10, 16
};

double getGravity() {
    return softDrop || level > 24 ? 20 : gravity[level];
}