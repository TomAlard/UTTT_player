#ifndef UTTT2_BOARD_H
#define UTTT2_BOARD_H

#include "square.h"
#include "../misc/player.h"
#include "player_bitboard.h"

#define TOTAL_SMALL_SQUARES 81
#define ANY_BOARD 9

typedef struct State {
    PlayerBitBoard player1;
    PlayerBitBoard player2;
    Player currentPlayer;
    uint8_t currentBoard;
    Winner winner;
    uint8_t ply;
} State;

typedef struct MCTSNode MCTSNode;

typedef struct Board {
    State state;
    State stateCheckpoint;
    MCTSNode* nodes;
    int currentNodeIndex;
    Player me;
} Board;

Board* createBoard();

void freeBoard(Board* board);

int allocateNodes(Board* board, uint8_t amount);

int8_t generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES]);

uint8_t getNextBoard(Board* board, uint8_t previousPosition);

bool nextBoardIsEmpty(Board* board);

void revertToCheckpoint(Board* board);

void updateCheckpoint(Board* board);

void makeTemporaryMove(Board* board, Square square);

void makePermanentMove(Board* board, Square square);

Winner getWinnerAfterMove(Board* board, Square square);

#endif //UTTT2_BOARD_H
