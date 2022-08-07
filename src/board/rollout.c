#include <assert.h>
#include "rollout.h"


void makeRolloutTemporaryMove(Board* board, RolloutState* RS, Square square) {
    assert(square.board == board->state.currentBoard
           || board->state.currentBoard == ANY_BOARD &&
              "Can't make a move on that board");
    assert(!squareIsOccupied(&board->state.player1, square) && !squareIsOccupied(&board->state.player2, square)
           && "Can't make a move on a square that is already occupied");
    assert(board->state.winner == NONE && "Can't make a move when there is already a winner");

    PlayerBitBoard* p1 = &board->state.player1;
    if (setSquareOccupied(p1 + board->state.currentPlayer, p1 + !board->state.currentPlayer, square)) {
        board->state.winner = winnerByBigBoards[board->state.player1.bigBoard][board->state.player2.bigBoard];
        board->state.totalAmountOfOpenSquares -= board->state.amountOfOpenSquaresBySmallBoard[square.board];
        board->state.amountOfOpenSquaresBySmallBoard[square.board] = 0;
        updateBigBoardState(board, RS);
    } else {
        board->state.totalAmountOfOpenSquares--;
        board->state.amountOfOpenSquaresBySmallBoard[square.board]--;
    }
    updateSmallBoardState(board, RS, square.board);
    board->state.currentPlayer ^= 1;
    board->state.currentBoard = getNextBoard(board, square.position);
    board->state.ply++;
}


void makeRandomTemporaryMove(Board* board, RolloutState* RS, RNG* rng) {
    assert(board->state.winner == NONE && "makeRandomTemporaryMove: there is already a winner");
    uint8_t currentBoard = board->state.currentBoard;
    if (hasWinningMove(board, RS)) {
        board->state.winner = board->state.currentPlayer + 1;
        return;
    }
    uint8_t randomMoveIndex;
    if (currentBoard == ANY_BOARD) {
        randomMoveIndex = generateBoundedRandomNumber(rng, board->state.totalAmountOfOpenSquares);
        currentBoard = 0;
        while (currentBoard < 9 && randomMoveIndex < 128) {
            randomMoveIndex -= board->state.amountOfOpenSquaresBySmallBoard[currentBoard++];
        }
        randomMoveIndex += board->state.amountOfOpenSquaresBySmallBoard[--currentBoard];
    } else {
        randomMoveIndex = generateBoundedRandomNumber(rng, board->state.amountOfOpenSquaresBySmallBoard[currentBoard]);
    }
    uint16_t bitBoard = ~(board->state.player1.smallBoards[currentBoard] | board->state.player2.smallBoards[currentBoard]) & 511;
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