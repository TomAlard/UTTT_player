#include <stdlib.h>
#include <assert.h>
#include "solver.h"

#define BIG_FLOAT 5000000.0f


void checkAllSet(MCTSNode* node) {
    if (node == NULL || node->amountOfUntriedMoves > 0) {
        return;
    }
    assert(node->sims < BIG_FLOAT || node->wins < BIG_FLOAT);
    bool hasDraw = false;
    for (int i = 0; i < node->amountOfChildren; i++) {
        MCTSNode child = node->children[i];
        if (child.sims < BIG_FLOAT || child.wins == child.sims) {
            return;
        }
        if (!hasDraw && child.wins > 0) {
            hasDraw = true;
        }
    }
    assert(node->sims < BIG_FLOAT || (node->wins / node->sims == 0.5f && hasDraw) || (node->wins == node->sims && !hasDraw));
    setNodeWinner(node, hasDraw? DRAW : WIN_P1, PLAYER1);
}


void setNodeWinner(MCTSNode* node, Winner winner, Player player) {
    if (node == NULL || node->sims >= BIG_FLOAT) {
        return;
    }
    node->sims = BIG_FLOAT;
    node->simsInverted = 1.0f / BIG_FLOAT;
    if (winner == DRAW) {
        node->wins = BIG_FLOAT / 2.0f;
        checkAllSet(node->parent);
    } else if (player + 1 == winner) {
        node->wins = BIG_FLOAT;
        setNodeWinner(node->parent, winner, OTHER_PLAYER(player));
    } else {
        node->wins = 0;
        checkAllSet(node->parent);
    }
}