#ifndef UTTT2_BOARD_H
#define UTTT2_BOARD_H

#include "square.h"
#include "player.h"

#define TOTAL_SMALL_SQUARES 81

typedef struct Board Board;

Board* createBoard();

void freeBoard(Board* bitBoard);

int generateMoves(Board* bitBoard, Square moves[TOTAL_SMALL_SQUARES]);

void makeMove(Board* bitBoard, Square square);

void revertToCheckpoint(Board* bitBoard);

void updateCheckpoint(Board* bitBoard);

Winner getWinner(Board* bitBoard);

#endif //UTTT2_BOARD_H
