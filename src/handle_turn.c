#include "handle_turn.h"
#include "misc/util.h"


int handleEnemyTurn(Board* board, int rootIndex, Square enemyMove) {
    if (enemyMove.board == 9 && enemyMove.position == 9) {
        board->me = PLAYER1;
        return rootIndex;
    }
    discoverChildNodes(rootIndex, board);
    int newRootIndex = updateRoot(&board->nodes[rootIndex], board, enemyMove);
    makePermanentMove(board, enemyMove);
    return newRootIndex;
}


HandleTurnResult handleTurn(Board* board, int rootIndex, double allocatedTime, Square enemyMove) {
    rootIndex = handleEnemyTurn(board, rootIndex, enemyMove);
    MCTSNode* root = &board->nodes[rootIndex];
    double time = board->state.ply <= 1? 10*allocatedTime : allocatedTime;
    int amountOfSimulations = findNextMove(board, rootIndex, time);
    Square move = getMostPromisingMove(board, root);
    int newRootIndex = updateRoot(root, board, move);
    makePermanentMove(board, move);
    HandleTurnResult result = {move, newRootIndex, amountOfSimulations};
    return result;
}
