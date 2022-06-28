#ifndef UTTT2_PLAYER_H
#define UTTT2_PLAYER_H

typedef enum Player {
    NONE,
    PLAYER1,
    PLAYER2,
    BOTH
} Player;

Player otherPlayer(Player player);

#endif //UTTT2_PLAYER_H
