#include <stdio.h>
#include "smart_rollout_tests.h"
#include "../../src/board/smart_rollout.h"
#include "../test_util.h"


void hasWinningMoveTest() {
    RNG rng;
    seedRNG(&rng, 69, 420);
    for (int _ = 0; _ < 100; _++) {
        RolloutState RS;
        initializeRolloutState(&RS);
        Board* board = createBoard();
        while (getWinner(board) == NONE) {
            int8_t amountOfMoves;
            Square movesArray[TOTAL_SMALL_SQUARES];
            Square* moves = generateMoves(board, movesArray, &amountOfMoves);
            bool winningMoveExists = false;
            Player currentPlayer = getCurrentPlayer(board);
            for (int i = 0; i < amountOfMoves; i++) {
                makeTemporaryMove(board, moves[i]);
                if (currentPlayer + 1 == getWinner(board)) {
                    winningMoveExists = true;
                }
                revertToCheckpoint(board);
            }
            myAssert((hasWinningMove(board, &RS) == winningMoveExists) || board->AS.totalAmountOfOpenSquares == 1);
            Square move = moves[generateBoundedRandomNumber(&rng, amountOfMoves)];
            PlayerBitBoard* player = currentPlayer == PLAYER1? &board->player1 : &board->player2;
            PlayerBitBoard* otherPlayer = currentPlayer == PLAYER1? &board->player2 : &board->player1;
            bool bigBoardWasUpdated = setSquareOccupied(player, otherPlayer, move);
            if (bigBoardWasUpdated) {
                updateBigBoardState(board, &RS);
            }
            updateSmallBoardState(&RS, move.board, board->player1.smallBoards[move.board], board->player2.smallBoards[move.board]);
            makePermanentMove(board, move);
        }
        freeBoard(board);
    }
}


void runSmartRolloutTests() {
    printf("\thasWinningMoveTest...\n");
    hasWinningMoveTest();
}