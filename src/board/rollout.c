#include <assert.h>
#include "rollout.h"


void makeRolloutTemporaryMove(Board* board, RolloutState* RS, Square square) {
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
        updateBigBoardState(RS, board->player1.bigBoard, board->player2.bigBoard);
    } else {
        board->AS.totalAmountOfOpenSquares--;
        board->AS.amountOfOpenSquaresBySmallBoard[square.board]--;
    }
    updateSmallBoardState(RS, square.board, board->player1.smallBoards[square.board], board->player2.smallBoards[square.board]);
    board->AS.currentPlayer ^= 1;
    board->AS.currentBoard = getNextBoard(board, square.position);
    board->AS.ply++;
}


void makeRandomTemporaryMove(Board* board, RolloutState* RS, RNG* rng) {
    assert(board->AS.winner == NONE && "makeRandomTemporaryMove: there is already a winner");
    uint8_t currentBoard = board->AS.currentBoard;
    if (hasWinningMove(RS, currentBoard, board->AS.currentPlayer)) {
        board->AS.winner = board->AS.currentPlayer + 1;
        return;
    }
    uint8_t randomMoveIndex;
    if (currentBoard == ANY_BOARD) {
        randomMoveIndex = generateBoundedRandomNumber(rng, board->AS.totalAmountOfOpenSquares);
        currentBoard = 0;
        while (currentBoard < 9 && randomMoveIndex < 128) {
            randomMoveIndex -= board->AS.amountOfOpenSquaresBySmallBoard[currentBoard++];
        }
        randomMoveIndex += board->AS.amountOfOpenSquaresBySmallBoard[--currentBoard];
    } else {
        randomMoveIndex = generateBoundedRandomNumber(rng, board->AS.amountOfOpenSquaresBySmallBoard[currentBoard]);
    }
    uint16_t bitBoard = ~(board->player1.smallBoards[currentBoard] | board->player2.smallBoards[currentBoard]) & 511;
    makeRolloutTemporaryMove(board, RS, openSquares[bitBoard][currentBoard][randomMoveIndex]);
}


Winner rollout(Board* board, RNG* rng) {
    RolloutState state;
    initializeRolloutState(&state);
    while (getWinner(board) == NONE) {
        makeRandomTemporaryMove(board, &state, rng);
    }
    return getWinner(board);
}