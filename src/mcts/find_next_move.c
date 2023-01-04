#include <sys/time.h>
#include "find_next_move.h"
#include "../misc/util.h"


MCTSNode* selectLeaf(Board* board, MCTSNode* root) {
    MCTSNode* currentNode = root;
    while (!isLeafNode(currentNode) && getWinner(board) == NONE) {
        currentNode = selectNextChild(currentNode);
        visitNode(currentNode, board);
    }
    return currentNode;
}


bool hasTimeRemaining(struct timeval start, double allocatedTime) {
    struct timeval end;
    gettimeofday(&end, NULL);
    double timePassed = (double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec);
    return timePassed < allocatedTime;
}


int findNextMove(Board* board, MCTSNode* root, double allocatedTime) {
    int amountOfSimulations = 0;
    struct timeval start;
    gettimeofday(&start, NULL);
    while (++amountOfSimulations % 128 != 0 || hasTimeRemaining(start, allocatedTime)) {
        MCTSNode* leaf = selectLeaf(board, root);
        Winner winner = getWinner(board);
        Player player = OTHER_PLAYER(getCurrentPlayer(board));
        if (winner == NONE) {
            backpropagateEval(expandLeaf(leaf, board));
        } else {
            backpropagate(leaf, winner, player);
        }
        revertToCheckpoint(board);
    }
    return amountOfSimulations;
}
