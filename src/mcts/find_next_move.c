#include <assert.h>
#include <sys/time.h>
#include "find_next_move.h"
#include "../misc/util.h"
#include "../board/rollout.h"
#include "solver.h"


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


bool hasTimeRemaining(struct timeval start, double allocatedTime) {
    struct timeval end;
    gettimeofday(&end, NULL);
    double timePassed = (double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec);
    return timePassed < allocatedTime;
}


int findNextMove(Board* board, MCTSNode* root, RNG* rng, double allocatedTime) {
    int amountOfSimulations = 0;
    struct timeval start;
    gettimeofday(&start, NULL);
    while (++amountOfSimulations % 128 != 0 || hasTimeRemaining(start, allocatedTime)) {
        MCTSNode* leaf = selectLeaf(board, root);
        MCTSNode* playoutNode;
        Winner simulationWinner;
        Winner winner = getWinner(board);
        Player player;
        if (winner == NONE) {
            playoutNode = expandLeaf(board, leaf);
            player = OTHER_PLAYER(getCurrentPlayer(board));
            simulationWinner = rollout(board, rng);
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
