#ifndef UTTT2_HANDLE_TURN_H
#define UTTT2_HANDLE_TURN_H

#include "mcts/find_next_move.h"

typedef struct HandleTurnResult {
    Square move;
    MCTSNode* newRoot;
    int amountOfSimulations;
} HandleTurnResult;

HandleTurnResult handleTurn(Board* board, MCTSNode* root, RNG* rng, double allocatedTime, Square enemyMove);

#endif //UTTT2_HANDLE_TURN_H
