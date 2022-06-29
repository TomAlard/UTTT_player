#ifndef UTTT2_PLAYER_H
#define UTTT2_PLAYER_H

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

typedef enum SmallBoardState {
    UNDECIDED,
    WON_P1,
    WON_P2,
    DRAW
} SmallBoardState;

#endif //UTTT2_PLAYER_H
