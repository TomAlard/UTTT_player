#include <stdio.h>
#include "profile_simulations.h"
#include "../src/handle_turn.h"


void profileSimulations() {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode();
    pcg32_random_t rng;
    pcg32_srandom_r(&rng, 69, 420);
    Square square = createSquare(1, 0);
    makePermanentMove(board, square);
    int totalSims = 0;
    for (int i = 0; i < 10; i++) {
        totalSims += findNextMove(board, root, &rng, 0.1);
    }
    printf("%d\n", totalSims / 10);
    freeMCTSTree(root);
    freeBoard(board);
}
