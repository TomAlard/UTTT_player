#include <assert.h>
#include "../test/tests_main.h"
#include "main.h"
#include "misc/util.h"
#include "arena/arena.h"


void skipMovesInput(FILE* file) {
    int validActionCount;
    int amountMatched = fscanf(file, "%d", &validActionCount);  // NOLINT(cert-err34-c)
    assert(amountMatched == 1 && "Incorrect amount of arguments matched");
    for (int i = 0; i < validActionCount; i++) {
        int row;
        int col;
        amountMatched = fscanf(file, "%d%d", &row, &col);  // NOLINT(cert-err34-c)
        assert(amountMatched == 2 && "Incorrect amount of arguments matched");
    }
}


void printMove(MCTSNode* root, Square bestMove, int amountOfSimulations) {
    Square s = toGameNotation(bestMove);
    uint8_t x = s.board;
    uint8_t y = s.position;
    float winrate = getWinrate(root);
    printf("%d %d %.4f %d\n", x, y, winrate, amountOfSimulations);
    fflush(stdout);
}


Square playTurn(Board* board, MCTSNode** root, RNG* rng, double allocatedTime, Square enemyMove) {
    HandleTurnResult result = handleTurn(board, *root, rng, allocatedTime, enemyMove);
    *root = result.newRoot;
    return result.move;
}


void playGame(FILE* file, double timePerMove) {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode();
    RNG rng;
    seedRNG(&rng, 69, 420);
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
        printMove(root, result.move, result.amountOfSimulations);
    }
    freeMCTSTree(root);
    freeBoard(board);
}


#define TIME 0.0999
int main() {
    // runTests();
    runArena();
    // playGame(stdin, TIME);
}
