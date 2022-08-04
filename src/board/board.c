#include <string.h>
#include <assert.h>
#include "board.h"
#include "../misc/util.h"
#include "smart_rollout.h"


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


Board* createBoard() {
    Board* board = safe_malloc(sizeof(Board));
    initializePlayerBitBoard(&board->player1);
    initializePlayerBitBoard(&board->player2);
    board->AS.currentPlayer = PLAYER1;
    board->AS.currentBoard = ANY_BOARD;
    board->AS.winner = NONE;
    board->AS.ply = 0;
    board->AS.totalAmountOfOpenSquares = 81;
    for (int boardIndex = 0; boardIndex < 9; boardIndex++) {
        board->AS.amountOfOpenSquaresBySmallBoard[boardIndex] = 9;
    }
    board->ASCheckpoint = board->AS;
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
    board->me = PLAYER2;
    initializeLookupTable();
    return board;
}


void freeBoard(Board* board) {
    safe_free(board);
}


Square* getMovesSingleBoard(Board* board, uint8_t boardIndex, int8_t* amountOfMoves) {
    uint16_t bitBoard = ~(board->player1.smallBoards[boardIndex] | board->player2.smallBoards[boardIndex]) & 511;
    *amountOfMoves = amountOfOpenSquares[bitBoard];
    return openSquares[bitBoard][boardIndex];
}


int8_t copyMovesSingleBoard(Board* board, uint8_t boardIndex, Square moves[TOTAL_SMALL_SQUARES], int8_t amountOfMoves) {
    uint16_t bitBoard = ~(board->player1.smallBoards[boardIndex] | board->player2.smallBoards[boardIndex]) & 511;
    memcpy(&moves[amountOfMoves], openSquares[bitBoard][boardIndex],
           amountOfOpenSquares[bitBoard] * sizeof(Square));
    return (int8_t)(amountOfMoves + amountOfOpenSquares[bitBoard]);
}


int8_t generateMovesAnyBoard(Board* board, Square moves[TOTAL_SMALL_SQUARES]) {
    int8_t amountOfMoves = 0;
    uint16_t undecidedSmallBoards = ~(board->player1.bigBoard | board->player2.bigBoard) & 511;
    while (undecidedSmallBoards) {
        uint8_t boardIndex = __builtin_ffs(undecidedSmallBoards) - 1;
        amountOfMoves = copyMovesSingleBoard(board, boardIndex, moves, amountOfMoves);
        undecidedSmallBoards &= undecidedSmallBoards - 1;
    }
    return amountOfMoves;
}


Square* generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves) {
    if (board->AS.winner != NONE) {
        *amountOfMoves = 0;
        return moves;
    }
    uint8_t currentBoard = board->AS.currentBoard;
    if (currentBoard == ANY_BOARD) {
        *amountOfMoves = generateMovesAnyBoard(board, moves);
        return moves;
    }
    return getMovesSingleBoard(board, currentBoard, amountOfMoves);
}


uint8_t getNextBoard(Board* board, uint8_t previousPosition) {
    bool smallBoardIsDecided = BIT_CHECK(board->player1.bigBoard | board->player2.bigBoard, previousPosition);
    return smallBoardIsDecided ? ANY_BOARD : previousPosition;
}


bool nextBoardIsEmpty(Board* board) {
    uint8_t currentBoard = board->AS.currentBoard;
    return currentBoard != ANY_BOARD
        && (board->player1.smallBoards[currentBoard] | board->player2.smallBoards[currentBoard]) == 0;
}


uint8_t getCurrentBoard(Board* board) {
    return board->AS.currentBoard;
}


Player getCurrentPlayer(Board* board) {
    return board->AS.currentPlayer;
}


void revertToCheckpoint(Board* board) {
    revertToPlayerCheckpoint(&board->player1);
    revertToPlayerCheckpoint(&board->player2);
    board->AS = board->ASCheckpoint;
}


void updateCheckpoint(Board* board) {
    updatePlayerCheckpoint(&board->player1);
    updatePlayerCheckpoint(&board->player2);
    board->ASCheckpoint = board->AS;
}


void makeTemporaryMove(Board* board, Square square) {
    assert(square.board == board->AS.currentBoard
            || board->AS.currentBoard == ANY_BOARD &&
            "Can't make a move on that board");
    assert(!squareIsOccupied(&board->player1, square) && !squareIsOccupied(&board->player2, square)
            && "Can't make a move on a square that is already occupied");
    assert(board->AS.winner == NONE && "Can't make a move when there is already a winner");

    PlayerBitBoard* p1 = &board->player1;
    if (setSquareOccupied(p1 + board->AS.currentPlayer, p1 + !board->AS.currentPlayer, square)) {
        board->AS.winner = winnerByBigBoards[board->player1.bigBoard][board->player2.bigBoard];
        board->AS.totalAmountOfOpenSquares -= board->AS.amountOfOpenSquaresBySmallBoard[square.board];
        board->AS.amountOfOpenSquaresBySmallBoard[square.board] = 0;
    } else {
        board->AS.totalAmountOfOpenSquares--;
        board->AS.amountOfOpenSquaresBySmallBoard[square.board]--;
    }
    board->AS.currentPlayer ^= 1;
    board->AS.currentBoard = getNextBoard(board, square.position);
    board->AS.ply++;
}


void makePermanentMove(Board* board, Square square) {
    makeTemporaryMove(board, square);
    updateCheckpoint(board);
}


Winner getWinner(Board* board) {
    return board->AS.winner;
}


void setMe(Board* board, Player player) {
    board->me = player;
}


bool currentPlayerIsMe(Board* board) {
    return board->AS.currentPlayer == board->me;
}


uint8_t getPly(Board* board) {
    return board->AS.ply;
}
