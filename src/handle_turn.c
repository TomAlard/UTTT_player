#include "handle_turn.h"


MCTSNode* handleEnemyTurn(Board* board, MCTSNode* root, Square enemyMove, RNG* rng) {
    if (enemyMove.board == 9 && enemyMove.position == 9) {
        setMe(board, PLAYER1);
        return root;
    }
    MCTSNode* newRoot = updateRoot(root, board, enemyMove, rng);
    makePermanentMove(board, enemyMove);
    return newRoot;
}


HandleTurnResult handleTurn(Board* board, MCTSNode* root, RNG* rng, double allocatedTime, Square enemyMove) {
    root = handleEnemyTurn(board, root, enemyMove, rng);
    int amountOfSimulations = findNextMove(board, root, rng, allocatedTime);
    Square move = getMostPromisingMove(root);
    MCTSNode* newRoot = updateRoot(root, board, move, rng);
    makePermanentMove(board, move);
    HandleTurnResult result = {move, newRoot, amountOfSimulations};
    return result;
}