#include <string.h>
#include <assert.h>
#include "board.h"
#include "../mcts/mcts_node.h"
#include "../misc/util.h"


Square openSquares[512][9][9];
int8_t amountOfOpenSquares[512];
Winner winnerByBigBoards[512][512];


int8_t setOpenSquares(Square openSquares_[9], uint8_t boardIndex, uint16_t bitBoard) {
    int8_t amountOfMoves = 0;
    while (bitBoard) {
        Square square = {boardIndex, __builtin_ffs(bitBoard) - 1};
        openSquares_[amountOfMoves++] = square;
        bitBoard &= bitBoard - 1;
    }
    return amountOfMoves;
}


Winner calculateWinner(uint16_t player1BigBoard, uint16_t player2BigBoard) {
    uint16_t decisiveBoards = player1BigBoard ^ player2BigBoard;
    uint16_t boardsWonByPlayer1 = player1BigBoard & decisiveBoards;
    if (isWin(boardsWonByPlayer1)) {
        return WIN_P1;
    }
    uint16_t boardsWonByPlayer2 = player2BigBoard & decisiveBoards;
    if (isWin(boardsWonByPlayer2)) {
        return WIN_P2;
    }
    if ((player1BigBoard | player2BigBoard) == 511) {
        int player1AmountBoardsWon = __builtin_popcount(player1BigBoard);
        int player2AmountBoardsWon = __builtin_popcount(player2BigBoard);
        return player1AmountBoardsWon > player2AmountBoardsWon
               ? WIN_P1
               : player1AmountBoardsWon < player2AmountBoardsWon
                 ? WIN_P2
                 : DRAW;
    }
    return NONE;
}


#define MEGABYTE (1024*1024ULL)
#define NODES_SIZE (512*MEGABYTE)
#define NUM_NODES (NODES_SIZE / sizeof(MCTSNode))
Board* createBoard() {
    Board* board = safeMalloc(sizeof(Board));
    initializePlayerBitBoard(&board->state.player1);
    initializePlayerBitBoard(&board->state.player2);
    board->state.currentPlayer = PLAYER1;
    board->state.currentBoard = ANY_BOARD;
    board->state.winner = NONE;
    board->state.ply = 0;
    board->stateCheckpoint = board->state;
    board->nodes = safeMalloc(NUM_NODES * sizeof(MCTSNode));
    board->currentNodeIndex = 0;
    board->me = PLAYER2;
    for (int boardIndex = 0; boardIndex < 9; boardIndex++) {
        for (int bitBoard = 0; bitBoard < 512; bitBoard++) {
            amountOfOpenSquares[bitBoard] =
                    setOpenSquares(openSquares[bitBoard][boardIndex], boardIndex, bitBoard);
        }
    }
    for (uint16_t player1BigBoard = 0; player1BigBoard < 512; player1BigBoard++) {
        for (uint16_t player2BigBoard = 0; player2BigBoard < 512; player2BigBoard++) {
            Winner winner = calculateWinner(player1BigBoard, player2BigBoard);
            winnerByBigBoards[player1BigBoard][player2BigBoard] = winner;
        }
    }
    return board;
}


void freeBoard(Board* board) {
    safeFree(board->nodes);
    safeFree(board);
}


int allocateNodes(Board* board, uint8_t amount) {
    int result = (amount > NUM_NODES - board->currentNodeIndex)? 0 : board->currentNodeIndex;
    board->currentNodeIndex = result + amount;
    return result;
}


Square* getMovesSingleBoard(Board* board, uint8_t boardIndex, int8_t* amountOfMoves) {
    uint16_t bitBoard = ~(board->state.player1.smallBoards[boardIndex] | board->state.player2.smallBoards[boardIndex]) & 511;
    *amountOfMoves = amountOfOpenSquares[bitBoard];
    return openSquares[bitBoard][boardIndex];
}


int8_t copyMovesSingleBoard(Board* board, uint8_t boardIndex, Square moves[TOTAL_SMALL_SQUARES], int8_t amountOfMoves) {
    uint16_t bitBoard = ~(board->state.player1.smallBoards[boardIndex] | board->state.player2.smallBoards[boardIndex]) & 511;
    memcpy(&moves[amountOfMoves], openSquares[bitBoard][boardIndex],
           amountOfOpenSquares[bitBoard] * sizeof(Square));
    return (int8_t)(amountOfMoves + amountOfOpenSquares[bitBoard]);
}


int8_t generateMovesAnyBoard(Board* board, Square moves[TOTAL_SMALL_SQUARES]) {
    int8_t amountOfMoves = 0;
    uint16_t undecidedSmallBoards = ~(board->state.player1.bigBoard | board->state.player2.bigBoard) & 511;
    while (undecidedSmallBoards) {
        uint8_t boardIndex = __builtin_ffs(undecidedSmallBoards) - 1;
        amountOfMoves = copyMovesSingleBoard(board, boardIndex, moves, amountOfMoves);
        undecidedSmallBoards &= undecidedSmallBoards - 1;
    }
    return amountOfMoves;
}


Square* generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves) {
    if (board->state.winner != NONE) {
        *amountOfMoves = 0;
        return moves;
    }
    uint8_t currentBoard = board->state.currentBoard;
    if (currentBoard == ANY_BOARD) {
        *amountOfMoves = generateMovesAnyBoard(board, moves);
        return moves;
    }
    return getMovesSingleBoard(board, currentBoard, amountOfMoves);
}


uint8_t getNextBoard(Board* board, uint8_t previousPosition) {
    bool smallBoardIsDecided = BIT_CHECK(board->state.player1.bigBoard | board->state.player2.bigBoard, previousPosition);
    return smallBoardIsDecided ? ANY_BOARD : previousPosition;
}


bool nextBoardIsEmpty(Board* board) {
    uint8_t currentBoard = board->state.currentBoard;
    return currentBoard != ANY_BOARD
        && (board->state.player1.smallBoards[currentBoard] | board->state.player2.smallBoards[currentBoard]) == 0;
}


uint8_t getCurrentBoard(Board* board) {
    return board->state.currentBoard;
}


Player getCurrentPlayer(Board* board) {
    return board->state.currentPlayer;
}


void revertToCheckpoint(Board* board) {
    board->state = board->stateCheckpoint;
}


void updateCheckpoint(Board* board) {
    board->stateCheckpoint = board->state;
}


void makeTemporaryMove(Board* board, Square square) {
    assert(square.board == board->state.currentBoard
           || board->state.currentBoard == ANY_BOARD &&
              "Can't make a move on that board");
    assert(!squareIsOccupied(&board->state.player1, square) && !squareIsOccupied(&board->state.player2, square)
           && "Can't make a move on a square that is already occupied");
    assert(board->state.winner == NONE && "Can't make a move when there is already a winner");

    PlayerBitBoard* p1 = &board->state.player1;
    if (setSquareOccupied(p1 + board->state.currentPlayer, p1 + !board->state.currentPlayer, square)) {
        board->state.winner = winnerByBigBoards[board->state.player1.bigBoard][board->state.player2.bigBoard];
    }
    board->state.currentPlayer ^= 1;
    board->state.currentBoard = getNextBoard(board, square.position);
    board->state.ply++;
}


void makePermanentMove(Board* board, Square square) {
    makeTemporaryMove(board, square);
    updateCheckpoint(board);
}


Winner getWinnerAfterMove(Board* board, Square square) {
    State tempCheckpoint = board->state;
    makeTemporaryMove(board, square);
    Winner winner = getWinner(board);
    board->state = tempCheckpoint;
    return winner;
}


Winner getWinner(Board* board) {
    return board->state.winner;
}


void setMe(Board* board, Player player) {
    board->me = player;
}


bool currentPlayerIsMe(Board* board) {
    return board->state.currentPlayer == board->me;
}


uint8_t getPly(Board* board) {
    return board->state.ply;
}
