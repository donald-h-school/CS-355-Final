#include <ncurses.h> 
#include <stdlib.h> 
#include <unistd.h> // for usleep()
#include <signal.h>
#include <time.h> 

struct Block {
    char position[4][4];
}

struct Block blocks[7];

blocks[0].position = {{'o','o'},
                      {'o','o'}};

blocks[1].position = {{' ',' ',' ',' '},
                      {'o','o','o','o'},
                      {' ',' ',' ',' '},
                      {' ',' ',' ',' '}};

blocks[2].position = {{' ',' ',' '},
                      {'o','o','o'},
                      {' ',' ','o'}};

blocks[3].position = {{' ',' ',' '},
                      {'o','o','o'},
                      {'o',' ',' '}};

blocks[4].position = {{' ',' ',' '},
                      {'o','o','o'},
                      {' ','o',' '}};

blocks[5].position = {{' ',' ',' '},
                      {'o','o',' '},
                      {' ','o','o'}};

blocks[6].position = {{' ',' ',' '},
                      {' ','o','o'},
                      {'o','o',' '}};
