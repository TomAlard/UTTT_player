#ifndef UTTT2_BITBOARD_H
#define UTTT2_BITBOARD_H

#include "square.h"
#include "player.h"

#define TOTAL_SMALL_SQUARES 81

typedef struct BitBoard BitBoard;

BitBoard* createBitBoard();

void freeBitBoard(BitBoard* bitBoard);

int generateMoves(BitBoard* bitBoard, Square moves[TOTAL_SMALL_SQUARES]);

Winner makeMove(BitBoard* bitBoard, Square square);

void revertToCheckpoint(BitBoard* bitBoard);

void updateCheckpoint(BitBoard* bitBoard);

#endif //UTTT2_BITBOARD_H
