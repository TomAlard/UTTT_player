#ifndef UTTTCODINGAMEC_MCTS_NODE_H
#define UTTTCODINGAMEC_MCTS_NODE_H

#include <stdbool.h>
#include "board.h"

typedef struct MCTSNode MCTSNode;

MCTSNode* init_MCTS_root_node(Player player);

void free_MCTS_tree(MCTSNode* root);

MCTSNode* get_next_child(MCTSNode* node, Board* board);

void backpropagate(MCTSNode* node, State state);

bool is_leaf_node(MCTSNode* node);

void visit_node(MCTSNode* node, Board* board);

void set_node_state(MCTSNode* node, State state);

double get_UCT_value(MCTSNode* node);

#endif //UTTTCODINGAMEC_MCTS_NODE_H
