#ifndef UTTT2_MCTS_NODE_H
#define UTTT2_MCTS_NODE_H

#include <stdbool.h>
#include "../board/board.h"

typedef struct MCTSNode {
    struct MCTSNode* parent;
    struct MCTSNode* children;
    float eval;
    float sims;
    float simsInverted;
    Square square;
    int8_t amountOfChildren;
} MCTSNode;

MCTSNode* createMCTSRootNode();

void freeMCTSTree(MCTSNode* root);

void discoverChildNodes(MCTSNode* node, Board* board, int gameId);

bool isLeafNode(MCTSNode* node);

MCTSNode* selectNextChild(MCTSNode* node);

MCTSNode* expandLeaf(MCTSNode* leaf, Board* board, int gameId);

MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square);

void backpropagate(MCTSNode* node, Winner winner, Player player);

void backpropagateEval(MCTSNode* node);

void visitNode(MCTSNode* node, Board* board);

Square getMostPromisingMove(MCTSNode* node);

float getEval(MCTSNode* node);

#endif //UTTT2_MCTS_NODE_H
