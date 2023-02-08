#include <assert.h>
#include "../test/tests_main.h"
#include "main.h"
#include "misc/util.h"
#include "arena/arena.h"


void skipMovesInput(FILE* file) {
    int validActionCount;
    fscanf(file, "%d", &validActionCount);  // NOLINT(cert-err34-c)
    for (int i = 0; i < validActionCount; i++) {
        int row;
        int col;
        fscanf(file, "%d%d", &row, &col);  // NOLINT(cert-err34-c)
    }
}


void printMove(MCTSNode* root, Square bestMove, int amountOfSimulations) {
    Square s = toGameNotation(bestMove);
    uint8_t x = s.board;
    uint8_t y = s.position;
    float winrate = root->eval;
    printf("%d %d %.4f %d\n", x, y, winrate, amountOfSimulations);
    fflush(stdout);
}


Square playTurn(Board* board, int* rootIndex, double allocatedTime, Square enemyMove) {
    HandleTurnResult result = handleTurn(board, *rootIndex, allocatedTime, enemyMove);
    *rootIndex = result.newRootIndex;
    return result.move;
}


void playGame(FILE* file, double timePerMove) {
    Board* board = createBoard();
    int rootIndex = createMCTSRootNode(board);
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
        HandleTurnResult result = handleTurn(board, rootIndex, timePerMove, enemyMove);
        rootIndex = result.newRootIndex;
        printMove(&board->nodes[rootIndex], result.move, result.amountOfSimulations);
    }
    freeBoard(board);
}


#define TIME 0.0999
int main() {
    runTests();
    // runArena();
    // playGame(stdin, TIME);
}
