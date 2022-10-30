#include <sys/time.h>
#include "find_next_move.h"
#include "../misc/util.h"
#include "../board/rollout.h"
#include "solver.h"


MCTSNode* selectLeaf(Board* board, MCTSNode* root, RNG* rng) {
    MCTSNode* currentNode = root;
    while (!isLeafNode(currentNode, board, rng) && getWinner(board) == NONE) {
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


int findNextMove(Board* board, MCTSNode* root, RNG* rng, double allocatedTime, int gameId) {
    int amountOfSimulations = 0;
    struct timeval start;
    gettimeofday(&start, NULL);
    while (++amountOfSimulations % 75000 != 0 || hasTimeRemaining(start, allocatedTime)) {
        MCTSNode* leaf = selectLeaf(board, root, rng);
        Winner winner = getWinner(board);
        Player player = OTHER_PLAYER(getCurrentPlayer(board));
        if (winner == NONE) {
            winner = rollout(board, rng, gameId);
            backpropagate(leaf, winner, player);
        } else {
            setNodeWinner(leaf, winner, player);
            backpropagate(leaf, winner, player);
        }
        revertToCheckpoint(board);
    }
    return amountOfSimulations;
}
