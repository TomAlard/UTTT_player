#include <stdlib.h>
#include <assert.h>
#include "solver.h"

#define BIG_FLOAT 10000000000000.0f


void checkAllSet(MCTSNode* node) {
    if (node == NULL || node->amountOfUntriedMoves > 0) {
        return;
    }
    assert(node->sims < BIG_FLOAT || node->wins < BIG_FLOAT);
    bool hasDraw = false;
    for (int i = 0; i < node->amountOfChildren; i++) {
        MCTSNode child = node->children[i];
        if (child.sims < BIG_FLOAT || child.wins == 0) {
            return;
        }
        if (!hasDraw && child.wins != child.sims) {
            hasDraw = true;
        }
    }
    node->sims = BIG_FLOAT;
    node->wins = hasDraw? BIG_FLOAT / 2.0f : 0;
    checkAllSet(node->parent);
}


void setNodeWinner(MCTSNode* node, Winner winner, Player player) {
    if (node->sims >= BIG_FLOAT) {
        return;
    }
    node->sims = BIG_FLOAT;
    if (winner == DRAW) {
        node->wins = BIG_FLOAT / 2.0f;
        checkAllSet(node->parent);
    } else if (player + 1 == winner) {
        node->wins = BIG_FLOAT;
        checkAllSet(node->parent);
    } else {
        node->wins = 0;
        if (node->parent != NULL) {
            node->parent->wins = BIG_FLOAT;
            checkAllSet(node->parent->parent);
        }
    }
}