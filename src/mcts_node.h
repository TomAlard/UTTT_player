#ifndef UTTT2_MCTS_NODE_H
#define UTTT2_MCTS_NODE_H

#include <stdbool.h>
#include "bitboard.h"

typedef struct MCTSNode MCTSNode;

MCTSNode* createMCTSRootNode();

void freeMCTSTree(MCTSNode* root);

bool hasChildren(MCTSNode* node, BitBoard* board);

bool isLeafNode(MCTSNode* node);

MCTSNode* selectNextChild(MCTSNode* node, BitBoard* board);

MCTSNode* updateRoot(MCTSNode* root, BitBoard* board, Square square);

void backpropagate(MCTSNode* node, Winner winner);

void visitNode(MCTSNode* node, BitBoard* board);

void setNodeWinner(MCTSNode* node, Winner winner);

Square getMostSimulatedChildSquare(MCTSNode* node);

int getSims(MCTSNode* node);

double getWinrate(MCTSNode* node);

#endif //UTTT2_MCTS_NODE_H
