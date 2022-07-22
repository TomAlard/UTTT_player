#include <time.h>
#include "find_next_move.h"
#include "util.h"


MCTSNode* selectLeaf(Board* board, MCTSNode* root) {
    MCTSNode* currentNode = root;
    while (!isLeafNode(currentNode, board) && getWinner(board) == NONE) {
        currentNode = selectNextChild(currentNode, board);
        assertMsg(currentNode != NULL, "selectLeaf: currentNode is NULL!");
        visitNode(currentNode, board);
    }
    return currentNode;
}


MCTSNode* expandLeaf(Board* board, MCTSNode* leaf) {
    MCTSNode* nextChild = selectNextChild(leaf, board);
    visitNode(nextChild, board);
    return nextChild;
}


Winner simulate(Board* board, pcg32_random_t* rng) {
    while (getWinner(board) == NONE) {
        Square movesArray[TOTAL_SMALL_SQUARES];
        int8_t amountOfMoves;
        Square* validMoves = generateMoves(board, movesArray, &amountOfMoves);
        Square move = validMoves[pcg32_boundedrand_r(rng, amountOfMoves)];
        makeTemporaryMove(board, move);
    }
    Winner simulationWinner = getWinner(board);
    revertToCheckpoint(board);
    return simulationWinner;
}


clock_t getDeadline(double time) {
    return clock() + (clock_t)(time*CLOCKS_PER_SEC);
}


bool hasTimeRemaining(clock_t deadline) {
    return clock() < deadline;
}


int findNextMove(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime) {
    clock_t deadline = getDeadline(allocatedTime);
    int amountOfSimulations = 0;
    while (++amountOfSimulations % 128 != 0 || hasTimeRemaining(deadline)) {
        MCTSNode* leaf = selectLeaf(board, root);
        MCTSNode* playoutNode;
        Winner simulationWinner;
        Winner winner = getWinner(board);
        if (winner == NONE) {
            playoutNode = expandLeaf(board, leaf);
            simulationWinner = simulate(board, rng);
        } else {
            playoutNode = leaf;
            simulationWinner = winner;
            setNodeWinner(playoutNode, winner);
            revertToCheckpoint(board);
        }
        backpropagate(playoutNode, simulationWinner);
    }
    return amountOfSimulations;
}
