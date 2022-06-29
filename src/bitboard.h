#ifndef UTTT2_BITBOARD_H
#define UTTT2_BITBOARD_H

#include "square.h"
#include "player.h"

typedef struct BitBoard BitBoard;

BitBoard* createBitBoard();

void freeBitBoard(BitBoard* bitBoard);

void makeMove(BitBoard* bitBoard, Square square);

void revertToCheckpoint(BitBoard* bitBoard);

void updateCheckpoint(BitBoard* bitBoard);

void clearBoard(BitBoard* bitBoard);

#endif //UTTT2_BITBOARD_H
