#ifndef UTTT2_MCTS_NODE_H
#define UTTT2_MCTS_NODE_H

#include <stdbool.h>
#include "../board/board.h"

typedef struct MCTSNode {
    int childrenIndex;
    float eval;
    float evalSum;
    float sims;
    Square square;
    int8_t amountOfChildren;
} MCTSNode;

int createMCTSRootNode(Board* board);

void discoverChildNodes(int nodeIndex, Board* board);

bool isLeafNode(int nodeIndex, Board* board);

int selectNextChild(Board* board, int nodeIndex);

MCTSNode* expandLeaf(int leafIndex, Board* board);

int updateRoot(MCTSNode* root, Board* board, Square square);

void backpropagate(Board* board, int nodeIndex, Winner winner, Player player, const int* parentIndices);

void backpropagateEval(Board* board, MCTSNode* node, const int* parentIndices);

void visitNode(int nodeIndex, Board* board);

Square getMostPromisingMove(Board* board, MCTSNode* node);

#endif //UTTT2_MCTS_NODE_H
