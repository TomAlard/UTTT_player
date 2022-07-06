#ifndef UTTT2_MCTS_NODE_H
#define UTTT2_MCTS_NODE_H

#include <stdbool.h>
#include "board.h"

typedef struct MCTSNode MCTSNode;

MCTSNode* createMCTSRootNode();

void freeMCTSTree(MCTSNode* root);

bool isLeafNode(MCTSNode* node, Board* board);

MCTSNode* selectNextChild(MCTSNode* node, Board* board);

MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square);

void backpropagate(MCTSNode* node, Winner winner);

void visitNode(MCTSNode* node, Board* board);

void setNodeWinner(MCTSNode* node, Winner winner);

Square getMostPromisingMove(MCTSNode* node, Board* board);

int getSims(MCTSNode* node);

double getWinrate(MCTSNode* node);

#endif //UTTT2_MCTS_NODE_H
