#ifndef UTTT2_PLAYER_BITBOARD_H
#define UTTT2_PLAYER_BITBOARD_H

#include "square.h"
#include "player.h"
#include <stdbool.h>

typedef struct PlayerBitBoard PlayerBitBoard;

PlayerBitBoard* createPlayerBitBoard();

void freePlayerBitBoard(PlayerBitBoard* playerBitBoard);

bool getPlayerBigBoardSquare(PlayerBitBoard* playerBitBoard, uint8_t board);

Player getPlayerSquare(PlayerBitBoard* playerBitBoard, Square square);

void setPlayerSquare(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayerBitBoard, Square square);

void clearPlayerSquare(PlayerBitBoard* playerBitBoard, Square square);

void revertToPlayerCheckpoint(PlayerBitBoard* playerBitBoard);

void updatePlayerCheckpoint(PlayerBitBoard* playerBitBoard);

void clearPlayerBoard(PlayerBitBoard* playerBitBoard);

#endif //UTTT2_PLAYER_BITBOARD_H
