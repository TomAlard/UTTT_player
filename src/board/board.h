#ifndef UTTT2_BOARD_H
#define UTTT2_BOARD_H

#include "square.h"
#include "../misc/player.h"
#include "../misc/random.h"
#include "smart_rollout.h"
#include "player_bitboard.h"

#define TOTAL_SMALL_SQUARES 81
#define ANY_BOARD 9

typedef struct AdditionalState {
    Player currentPlayer;
    uint8_t currentBoard;
    Winner winner;
    uint8_t ply;
    uint8_t totalAmountOfOpenSquares;
    uint8_t amountOfOpenSquaresBySmallBoard[9];
} AdditionalState;

typedef struct Board {
    PlayerBitBoard player1;
    PlayerBitBoard player2;
    AdditionalState AS;
    AdditionalState ASCheckpoint;
    Player me;
} Board;

Square openSquares[512][9][9];
int8_t amountOfOpenSquares[512];
Winner winnerByBigBoards[512][512];

typedef struct Board Board;

Board* createBoard();

void freeBoard(Board* board);

Square* generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves);

uint8_t getNextBoard(Board* board, uint8_t previousPosition);

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
