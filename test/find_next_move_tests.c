#include <stdio.h>
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
        Square nextMove = findNextMove(board, root, 0.005, &rng);
        myAssert(winnerBefore == getWinner(board));
        Square movesAfter[TOTAL_SMALL_SQUARES];
        int amountMovesAfter = generateMoves(board, movesAfter);
        myAssert(amountMovesBefore == amountMovesAfter);
        for (int i = 0; i < amountMovesBefore; i++) {
            myAssert(squaresAreEqual(movesBefore[i], movesAfter[i]));
        }
        makeMove(board, nextMove);
        updateCheckpoint(board);
        root = updateRoot(root, board, nextMove);
    }
    freeMCTSTree(root);
    freeBoard(board);
}


void runFindNextMoveTests() {
    printf("\tfindNextMoveDoesNotChangeBoard...\n");
    findNextMoveDoesNotChangeBoard();
}