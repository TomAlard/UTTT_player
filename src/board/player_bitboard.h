#ifndef UTTT2_PLAYER_BITBOARD_H
#define UTTT2_PLAYER_BITBOARD_H

#include "square.h"
#include "../misc/player.h"
#include <stdbool.h>

typedef struct PlayerBitBoard {
    __uint128_t marks;
    uint16_t bigBoard;
} PlayerBitBoard;

void initializePlayerBitBoard(PlayerBitBoard* playerBitBoard);

bool isWin(uint16_t smallBoard);

bool isDraw(uint16_t smallBoard, uint16_t otherPlayerSmallBoard);

bool setSquareOccupied(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayerBitBoard, Square square);

uint16_t extractSmallBoard(PlayerBitBoard* playerBitBoard, uint8_t smallBoardIndex);

#endif //UTTT2_PLAYER_BITBOARD_H
