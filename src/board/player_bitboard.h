#ifndef UTTT2_PLAYER_BITBOARD_H
#define UTTT2_PLAYER_BITBOARD_H

#include "square.h"
#include "../misc/player.h"
#include <stdbool.h>

typedef struct PlayerBitBoard {
    uint16_t smallBoards[9];
    uint16_t bigBoard;
    uint16_t checkpointSmallBoards[9];
    uint16_t checkpointBigBoard;
} PlayerBitBoard;

PlayerBitBoard* createPlayerBitBoard();

void initializePlayerBitBoard(PlayerBitBoard* playerBitBoard);

void freePlayerBitBoard(PlayerBitBoard* playerBitBoard);

bool boardIsWon(PlayerBitBoard* playerBitBoard, uint8_t board);

bool squareIsOccupied(PlayerBitBoard* playerBitBoard, Square square);

bool isWin(uint16_t smallBoard);

bool setSquareOccupied(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayerBitBoard, Square square);

void revertToPlayerCheckpoint(PlayerBitBoard* playerBitBoard);

void updatePlayerCheckpoint(PlayerBitBoard* playerBitBoard);

#endif //UTTT2_PLAYER_BITBOARD_H
