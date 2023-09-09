#include <sys/time.h>
#include "find_next_move.h"
#include "../misc/util.h"


int selectLeaf(Board* board, int rootIndex, int* parentIndices, int* i) {
    *i = TOTAL_SMALL_SQUARES - 1;
    parentIndices[(*i)--] = -1;
    int currentNodeIndex = rootIndex;
    while (!isLeafNode(currentNodeIndex, board) && board->state.winner == NONE) {
        parentIndices[(*i)--] = currentNodeIndex;
        currentNodeIndex = selectNextChild(board, currentNodeIndex);
        visitNode(currentNodeIndex, board);
    }
    return currentNodeIndex;
}


bool hasTimeRemaining(struct timeval start, double allocatedTime) {
    struct timeval end;
    gettimeofday(&end, NULL);
    double timePassed = (double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec);
    return timePassed < allocatedTime;
}


int findNextMove(Board* board, int rootIndex, double allocatedTime) {
    int amountOfSimulations = 0;
    struct timeval start;
    gettimeofday(&start, NULL);
    while (++amountOfSimulations % 512 != 0 || hasTimeRemaining(start, allocatedTime)) {
        int parentIndicesArray[TOTAL_SMALL_SQUARES];
        int i;
        int leafIndex = selectLeaf(board, rootIndex, parentIndicesArray, &i);
        int* parentIndices = &parentIndicesArray[i + 1];
        Winner winner = board->state.winner;
        Player player = OTHER_PLAYER(board->state.currentPlayer);
        if (winner == NONE) {
            backpropagateEval(board, expandLeaf(leafIndex, board), parentIndices);
        } else {
            backpropagate(board, leafIndex, winner, player, parentIndices);
        }
        revertToCheckpoint(board);
    }
    return amountOfSimulations;
}
