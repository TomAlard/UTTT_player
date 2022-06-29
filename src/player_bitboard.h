#ifndef UTTT2_PLAYER_BITBOARD_H
#define UTTT2_PLAYER_BITBOARD_H

#include "square.h"
#include "player.h"
#include <stdbool.h>

typedef struct PlayerBitBoard PlayerBitBoard;

PlayerBitBoard* createPlayerBitBoard();

void freePlayerBitBoard(PlayerBitBoard* playerBitBoard);

bool boardIsWon(PlayerBitBoard* playerBitBoard, uint8_t board);

bool squareIsOccupied(PlayerBitBoard* playerBitBoard, Square square);

void setSquareOccupied(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayerBitBoard, Square square);

void revertToPlayerCheckpoint(PlayerBitBoard* playerBitBoard);

void updatePlayerCheckpoint(PlayerBitBoard* playerBitBoard);

#endif //UTTT2_PLAYER_BITBOARD_H
