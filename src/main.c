#include <stdio.h>
#include "../test/tests_main.h"
#include "handle_turn.h"
#include "util.h"


void skipMovesInput(FILE* file) {
    int validActionCount;
    int amountMatched = fscanf(file, "%d", &validActionCount);  // NOLINT(cert-err34-c)
    assertMsg(amountMatched == 1, "Incorrect amount of arguments matched");
    for (int i = 0; i < validActionCount; i++) {
        int row;
        int col;
        amountMatched = fscanf(file, "%d%d", &row, &col);  // NOLINT(cert-err34-c)
        assertMsg(amountMatched == 1, "Incorrect amount of arguments matched");
    }
}


void printMove(MCTSNode* root, Square bestMove) {
    Square s = toGameNotation(bestMove);
    uint8_t x = s.board;
    uint8_t y = s.position;
    double winrate = getWinrate(root);
    int sims = getSims(root);
    printf("%d %d %.4f %d\n", x, y, winrate, sims);
    fflush(stdout);
}


void playGame(FILE* file, double timePerMove) {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode();
    pcg32_random_t rng;
    pcg32_srandom_r(&rng, 69, 420);
    while (true) {
        int enemy_row;
        int enemy_col;
        int v = fscanf(file, "%d%d", &enemy_row, &enemy_col);  // NOLINT(cert-err34-c)
        if (v == EOF) {
            break;
        }
        skipMovesInput(file);
        Square enemyMoveGameNotation = {enemy_row, enemy_col};
        Square enemyMove = toOurNotation(enemyMoveGameNotation);
        HandleTurnResult result = handleTurn(board, root, &rng, timePerMove, enemyMove);
        root = result.newRoot;
        printMove(root, result.move);
    }
    freeMCTSTree(root);
    freeBoard(board);
}


#define TIME 0.0999
int main() {
    // runTests();
    playGame(stdin, TIME);
}
