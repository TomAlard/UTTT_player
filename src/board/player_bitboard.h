#ifndef UTTT2_PLAYER_BITBOARD_H
#define UTTT2_PLAYER_BITBOARD_H

#include "square.h"
#include "../misc/player.h"
#include <stdbool.h>

typedef struct PlayerBitBoard {
    uint16_t smallBoards[9];
    uint16_t bigBoard;
} PlayerBitBoard;

void initializePlayerBitBoard(PlayerBitBoard* playerBitBoard);

bool boardIsWon(PlayerBitBoard* playerBitBoard, uint8_t board);

bool squareIsOccupied(PlayerBitBoard* playerBitBoard, Square square);

bool isWin(uint16_t smallBoard);

bool isDraw(uint16_t smallBoard, uint16_t otherPlayerSmallBoard);

bool setSquareOccupied(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayerBitBoard, Square square);

#endif //UTTT2_PLAYER_BITBOARD_H
