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
        Square nextMove = findNextMove(board, root, &rng, 0.005);
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
        struct timeval t1, t2;
        long elapsedTime;
        int timeMs = 5;
        gettimeofday(&t1, NULL);
        Square nextMove = findNextMove(board, root, &rng, 5 / 1000.0);
        gettimeofday(&t2, NULL);
        elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000;
        elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000;
        myAssert(elapsedTime == timeMs);
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