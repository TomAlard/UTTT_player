#include <stdlib.h>
#include <math.h>
#include "mcts_node.h"
#include "util.h"


typedef struct MCTSNode {
    MCTSNode* parent;
    Square position;
    Player player;
    double wins;
    int sims;
    double UCT_value;
    int amount_of_children;
    MCTSNode** children;
} MCTSNode;


MCTSNode* init_MCTS_root_node(Player player) {
    MCTSNode* root = safe_malloc(sizeof(MCTSNode));
    root->parent = NULL;
    root->position.grid = -1;
    root->position.pos = -1;
    root->player = player;
    root->wins = 0;
    root->sims = 0;
    root->UCT_value = -1;
    root->amount_of_children = -1;
    root->children = NULL;
    return root;
}


MCTSNode* init_MCTS_node(MCTSNode* parent, Square position, Player player) {
    MCTSNode* node = safe_malloc(sizeof(MCTSNode));
    node->parent = parent;
    node->position = position;
    node->player = player;
    node->wins = 0;
    node->sims = 0;
    node->UCT_value = -1;
    node->amount_of_children = -1;
    node->children = NULL;
    return node;
}


void free_MCTS_tree(MCTSNode* root) {
    if (root->children != NULL) {
        for (int i = 0; i < root->amount_of_children; i++) {
            free_MCTS_tree(root->children[i]);
        }
    }
    free(root->children);
    free(root);
}


void find_child_nodes(MCTSNode* node, Board* board) {
    Square moves_buffer[TOTAL_SQUARES];
    int amount_of_moves = get_possible_moves(board, moves_buffer);
    node->children = safe_malloc(amount_of_moves * sizeof(MCTSNode*));
    for (int i = 0; i < amount_of_moves; i++) {
        node->children[i] = init_MCTS_node(node, moves_buffer[i], other_player(node->player));
    }
    node->amount_of_children = amount_of_moves;
}


bool has_children(MCTSNode* node, Board* board) {
    if (node->children == NULL) {
        find_child_nodes(node, board);
    }
    return node->amount_of_children > 0;
}


MCTSNode* get_next_child_by_UCT(MCTSNode* node, Board* board) {
    if (node->children == NULL) {
        find_child_nodes(node, board);
    }

    MCTSNode* highest_UCT_child = NULL;
    double highest_UCT = -1000000000;
    for (int i = 0; i < node->amount_of_children; i++) {
        MCTSNode* child = node->children[i];
        double UCT = get_UCT_value(child);
        if (UCT > highest_UCT) {
            highest_UCT_child = child;
            highest_UCT = UCT;
        }
    }
    return highest_UCT_child;
}


MCTSNode* get_next_root(MCTSNode* root, Board* board, Square square) {
    if (root->children == NULL) {
        find_child_nodes(root, board);
    }

    for (int i = 0; i < root->amount_of_children; i++) {
        MCTSNode* child = root->children[i];
        if (squares_are_equal(square, child->position)) {
            child->parent = NULL;
            return child;
        }
    }
    return NULL;
}


Square get_most_simulated_child_square(MCTSNode* node) {
    MCTSNode* highest_sims_child = NULL;
    int highest_sims = -1;
    for (int i = 0; i < node->amount_of_children; i++) {
        MCTSNode* child = node->children[i];
        int sims = child->sims;
        if (sims > highest_sims) {
            highest_sims_child = child;
            highest_sims = sims;
        }
    }
    return highest_sims_child->position;
}


double get_wins(MCTSNode* node) {
    return node->wins;
}


int get_sims(MCTSNode* node) {
    return node->sims;
}


bool winner_equals_player(State state, Player player) {
    return (state == PLAYER_1_WIN && player == PLAYER_1) || (state == PLAYER_2_WIN && player == PLAYER_2);
}


void backpropagate(MCTSNode* node, State state) {
    node->sims++;
    if (winner_equals_player(state, node->player)) {
        node->wins++;
    } else if (state == DRAW) {
        node->wins += 0.5;
    }

    if (node->parent != NULL) {
        backpropagate(node->parent, state);
    }
}


bool is_leaf_node(MCTSNode* node) {
    return node->sims == 0;
}


void visit_node(MCTSNode* node, Board* board) {
    make_temporary_move(board, node->position);
}


#define UCT_WIN 100000
#define UCT_LOSS -UCT_WIN
void set_node_state(MCTSNode* node, State state) {
    if (state != DRAW) {
        bool win = winner_equals_player(state, node->player);
        node->UCT_value = win? UCT_WIN : UCT_LOSS;
        if (!win) {
            node->parent->UCT_value = UCT_WIN;
        }
    }
}


#define EXPLORATION_PARAMETER 1.41
double get_UCT_value(MCTSNode* node) {
    if (node->UCT_value != -1) {
        return node->UCT_value;
    }

    double w = node->wins;
    double n = node->sims != 0? node->sims : 0.0001;
    double c = EXPLORATION_PARAMETER;
    double N = node->parent->sims != 0? node->parent->sims : 0.0001;
    return w/n + c*(log(N) / n);
}
