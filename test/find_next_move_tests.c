#include <stdio.h>
#include <sys/time.h>
#include "find_next_move_tests.h"
#include "../src/find_next_move.h"
#include "test_util.h"


void findNextMoveDoesNotChangeBoard() {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode();
    pcg32_random_t rng;
    pcg32_srandom_r(&rng, 69, 420);
    while (getWinner(board) == NONE) {
        Winner winnerBefore = getWinner(board);
        Square movesBefore[TOTAL_SMALL_SQUARES];
        int amountMovesBefore = generateMoves(board, movesBefore);
        findNextMove(board, root, &rng, 0.005);
        Square nextMove = getMostSimulatedChildSquare(root);
        myAssert(winnerBefore == getWinner(board));
        Square movesAfter[TOTAL_SMALL_SQUARES];
        int amountMovesAfter = generateMoves(board, movesAfter);
        myAssert(amountMovesBefore == amountMovesAfter);
        for (int i = 0; i < amountMovesBefore; i++) {
            myAssert(squaresAreEqual(movesBefore[i], movesAfter[i]));
        }
        makePermanentMove(board, nextMove);
        root = updateRoot(root, board, nextMove);
    }
    freeMCTSTree(root);
    freeBoard(board);
}


void findNextMoveUsesAsMuchTimeAsWasGiven() {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode();
    pcg32_random_t rng;
    pcg32_srandom_r(&rng, 69, 420);
    while (getWinner(board) == NONE) {
        struct timeval start, end;
        int timeMs = 50;
        gettimeofday(&start, NULL);
        findNextMove(board, root, &rng, timeMs / 1000.0);
        Square nextMove = getMostSimulatedChildSquare(root);
        gettimeofday(&end, NULL);
        double elapsedTime = (double) (end.tv_sec - start.tv_sec) * 1000.0;
        elapsedTime += (double) (end.tv_usec - start.tv_usec) / 1000.0;
        myAssert(elapsedTime >= timeMs);
        if (elapsedTime >= 1.1*timeMs) {
            printf("%f\n", elapsedTime);
        }
        makePermanentMove(board, nextMove);
        root = updateRoot(root, board, nextMove);
    }
    freeMCTSTree(root);
    freeBoard(board);
}


void runFindNextMoveTests() {
    printf("\tfindNextMoveDoesNotChangeBoard...\n");
    findNextMoveDoesNotChangeBoard();
    printf("\tfindNextMoveUsesAsMuchTimeAsWasGiven...\n");
    findNextMoveUsesAsMuchTimeAsWasGiven();
}