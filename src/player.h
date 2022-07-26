#ifndef UTTT2_PLAYER_H
#define UTTT2_PLAYER_H

#include <stdbool.h>

#define Player bool
#define PLAYER1 0
#define PLAYER2 1
#define OTHER_PLAYER(p) ((p)^1)

typedef enum Winner {
    NONE,
    WIN_P1,
    WIN_P2,
    DRAW
} Winner;

#endif //UTTT2_PLAYER_H
