#ifndef UTTT2_BOARD_H
#define UTTT2_BOARD_H

#include "square.h"
#include "../misc/player.h"
#include "../misc/random.h"
#include "smart_rollout.h"

#define TOTAL_SMALL_SQUARES 81

typedef struct Board Board;

Board* createBoard();

void freeBoard(Board* board);

Square* generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves);

void makeRandomTemporaryMove(Board* board, RolloutState* RS, RNG* rng);

bool nextBoardIsEmpty(Board* board);

uint8_t getCurrentBoard(Board* board);

Player getCurrentPlayer(Board* board);

void revertToCheckpoint(Board* board);

void makeTemporaryMove(Board* board, Square square);

void makePermanentMove(Board* board, Square square);

Winner getWinner(Board* board);

void setMe(Board* board, Player player);

bool currentPlayerIsMe(Board* board);

uint8_t getPly(Board* board);

#endif //UTTT2_BOARD_H