#include "handle_turn.h"


MCTSNode* handleEnemyTurn(Board* board, MCTSNode* root, Square enemyMove) {
    if (enemyMove.board == 9 && enemyMove.position == 9) {
        return root;
    }
    MCTSNode* newRoot = updateRoot(root, board, enemyMove);
    makePermanentMove(board, enemyMove);
    return newRoot;
}


HandleTurnResult handleTurn(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime, Square enemyMove) {
    root = handleEnemyTurn(board, root, enemyMove);
    int amountOfSimulations = findNextMove(board, root, rng, allocatedTime);
    Square move = getMostPromisingMove(root, board);
    MCTSNode* newRoot = updateRoot(root, board, move);
    makePermanentMove(board, move);
    HandleTurnResult result = {move, newRoot, amountOfSimulations};
    return result;
}