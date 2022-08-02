#ifndef UTTT2_PLAYER_H
#define UTTT2_PLAYER_H

#include <stdbool.h>

#define Player bool
#define PLAYER1 0
#define PLAYER2 1
#define OTHER_PLAYER(p) ((p)^1)

#define Winner uint8_t
#define NONE 0
#define WIN_P1 1
#define WIN_P2 2
#define DRAW 3

#endif //UTTT2_PLAYER_H
