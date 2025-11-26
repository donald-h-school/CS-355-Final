#include <ncurses.h> 
#include <stdlib.h> 
#include <unistd.h> // for usleep()
#include <signal.h>
#include <time.h> 

struct Block {
    char structure[4][4];
};

struct Block cube = {.structure = {{'O','O'},
                                   {'O','O'}}};

struct Block line = {.structure = {{' ',' ',' ',' '},
                                   {'O','O','O','O'},
                                   {' ',' ',' ',' '},
                                   {' ',' ',' ',' '}}};

struct Block reverseL = {.structure = {{' ',' ',' '},
                                       {'O','O','O'},
                                       {' ',' ','O'}}};

struct Block regL = {.structure = {{' ',' ',' '},
                                   {'O','O','O'},
                                   {'O',' ',' '}}};

struct Block tee = {.structure = {{' ',' ',' '},
                                  {'O','O','O'},
                                  {' ','O',' '}}};

struct Block regZ = {.structure = {{' ',' ',' '},
                                   {'O','O',' '},
                                   {' ','O','O'}}};

struct Block reverseZ = {.structure = {{' ',' ',' '},
                                       {' ','O','O'},
                                       {'O','O',' '}}};

struct Block current;

int main() {
    
}

void rotate() {
    int a = sizeof(current.structure);
    int b = sizeof(current.structure[0]);
    for (int i = 0; i < a; i++) {
        for (int j = i + 1; j < b; j++) {
            int temp = current.structure[i][j];
            current.structure[i][j] = current.structure[j][i];
            current.structure[j][i] = temp;
        }
    }
    for (int i = 0; i < a; i++) {
        char temp[4];
        int k = 0;
        for (int j = b - 1; j >= 0; j--) {
            temp[j] = current.structure[i][k];
        }
        for (int j = 0; j < b; j++) {
            current.structure[i][j] = temp[j];
        }
    }
}
