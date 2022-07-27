#include <time.h>
#include <assert.h>
#include "find_next_move.h"
#include "util.h"


MCTSNode* selectLeaf(Board* board, MCTSNode* root) {
    MCTSNode* currentNode = root;
    while (!isLeafNode(currentNode, board) && getWinner(board) == NONE) {
        currentNode = selectNextChild(currentNode);
        assert(currentNode != NULL && "selectLeaf: currentNode is NULL!");
        visitNode(currentNode, board);
    }
    return currentNode;
}


MCTSNode* expandLeaf(Board* board, MCTSNode* leaf) {
    MCTSNode* nextChild = selectNextChild(leaf);
    visitNode(nextChild, board);
    return nextChild;
}


Winner simulate(Board* board, RNG* rng) {
    while (getWinner(board) == NONE) {
        makeRandomTemporaryMove(board, rng);
    }
    return getWinner(board);
}


clock_t getDeadline(double time) {
    return clock() + (clock_t)(time*CLOCKS_PER_SEC);
}


bool hasTimeRemaining(clock_t deadline) {
    return clock() < deadline;
}


int findNextMove(Board* board, MCTSNode* root, RNG* rng, double allocatedTime) {
    clock_t deadline = getDeadline(allocatedTime);
    int amountOfSimulations = 0;
    while (++amountOfSimulations % 128 != 0 || hasTimeRemaining(deadline)) {
        MCTSNode* leaf = selectLeaf(board, root);
        MCTSNode* playoutNode;
        Winner simulationWinner;
        Winner winner = getWinner(board);
        Player player;
        if (winner == NONE) {
            playoutNode = expandLeaf(board, leaf);
            player = OTHER_PLAYER(getCurrentPlayer(board));
            simulationWinner = simulate(board, rng);
        } else {
            playoutNode = leaf;
            simulationWinner = winner;
            player = OTHER_PLAYER(getCurrentPlayer(board));
            setNodeWinner(playoutNode, winner, player);
        }
        backpropagate(playoutNode, simulationWinner, player);
        revertToCheckpoint(board);
    }
    return amountOfSimulations;
}
