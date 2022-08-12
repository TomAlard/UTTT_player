#ifndef UTTT2_MCTS_NODE_H
#define UTTT2_MCTS_NODE_H

#include <stdbool.h>
#include "../board/board.h"

typedef struct MCTSNode {
    struct MCTSNode* parent;
    struct MCTSNode* children;
    float wins;
    float sims;
    float simsInverted;
    Square square;
    int8_t amountOfChildren;
    int8_t amountOfUntriedMoves;
} MCTSNode;

MCTSNode* createMCTSRootNode();

void freeMCTSTree(MCTSNode* root);

void discoverChildNodes(MCTSNode* node, Board* board, RNG* rng);

bool isLeafNode(MCTSNode* node, Board* board, RNG* rng);

MCTSNode* selectNextChild(MCTSNode* node);

MCTSNode* expandNextChild(MCTSNode* node);

MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square);

void backpropagate(MCTSNode* node, Winner winner, Player player);

void visitNode(MCTSNode* node, Board* board);

Square getMostPromisingMove(MCTSNode* node);

float getWinrate(MCTSNode* node);

#endif //UTTT2_MCTS_NODE_H
