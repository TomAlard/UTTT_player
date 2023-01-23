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
    float winrate = getEval(root);
    printf("%d %d %.4f %d\n", x, y, winrate, amountOfSimulations);
    fflush(stdout);
}


Square playTurn(Board* board, MCTSNode** root, double allocatedTime, Square enemyMove, int gameId) {
    HandleTurnResult result = handleTurn(board, *root, allocatedTime, enemyMove, gameId);
    *root = result.newRoot;
    return result.move;
}


void playGame(FILE* file, double timePerMove) {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode();
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
        HandleTurnResult result = handleTurn(board, root, timePerMove, enemyMove, -1);
        root = result.newRoot;
        printMove(root, result.move, result.amountOfSimulations);
    }
    freeMCTSTree(root);
    freeBoard(board);
}


#define TIME 0.0999
int main() {
    initializeBoardLookupTables();
    // runTests();
    runArena();
    // playGame(stdin, TIME);
}
