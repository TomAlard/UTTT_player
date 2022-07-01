#ifndef UTTT2_PLAYER_H
#define UTTT2_PLAYER_H

#include <stdbool.h>

typedef enum Player {
    PLAYER1,
    PLAYER2
} Player;

Player otherPlayer(Player player);

typedef enum Occupation {
    UNOCCUPIED,
    OCCUPIED_P1,
    OCCUPIED_P2
} Occupation;

typedef enum Winner {
    NONE,
    WIN_P1,
    WIN_P2,
    DRAW
} Winner;

bool playerIsWinner(Player player, Winner winner);

#endif //UTTT2_PLAYER_H
