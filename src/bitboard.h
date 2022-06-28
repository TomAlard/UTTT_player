#ifndef UTTT2_BITBOARD_H
#define UTTT2_BITBOARD_H

#include "square.h"
#include "player.h"

typedef struct BitBoard BitBoard;

BitBoard* createBitBoard();

void freeBitBoard(BitBoard* bitBoard);

Player getBigBoardSquare(BitBoard* bitBoard, uint8_t board);

Player getSquare(BitBoard* bitBoard, Square square);

void setSquare(BitBoard* bitBoard, Square square, Player player);

// DELETE THIS
void clearBoard(BitBoard* bitBoard);

#endif //UTTT2_BITBOARD_H
