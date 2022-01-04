#include <time.h>
#include "find_best_move.h"
#include "util.h"
#include "random.h"


MCTSNode* select_leaf(Board* board, MCTSNode* root) {
    MCTSNode* current_node = root;
    while (!is_leaf_node(current_node) && has_children(current_node, board)) {
        current_node = get_next_child_by_UCT(current_node, board);
        visit_node(current_node, board);
    }
    return current_node;
}


MCTSNode* expand_leaf(Board* board, MCTSNode* leaf) {
    MCTSNode* next_child = get_next_child_by_UCT(leaf, board);
    if (next_child != NULL) {
        visit_node(next_child, board);
        return next_child;
    }
    return leaf;
}


State simulate(Board* board, MCTSNode* playout_node, pcg32_random_t* rng) {
    Player current_player = other_player(get_player(playout_node));
    while (calculate_board_state(board) == UNDECIDED) {
        Square valid_moves[TOTAL_SQUARES];
        int amount_of_moves = get_possible_moves(board, valid_moves);
        Square move = valid_moves[pcg32_boundedrand_r(rng, amount_of_moves)];
        make_temporary_move(board, move);
        current_player = other_player(current_player);
    }
    State simulation_result = calculate_board_state(board);
    undo_all_temporary_moves(board);
    return simulation_result;
}


Square find_best_move(Board* board, MCTSNode* root, double allocated_time) {
    clock_t deadline = get_deadline(allocated_time);
    while (has_time_remaining(deadline)) {
        MCTSNode* leaf = select_leaf(board, root);
        MCTSNode* playout_node;
        State simulation_result;
        State board_state = calculate_board_state(board);
        if (board_state == UNDECIDED) {
            playout_node = expand_leaf(board, leaf);
            simulation_result = simulate(board, playout_node);
        } else {
            playout_node = leaf;
            simulation_result = board_state;
            set_node_state(playout_node, board_state);
        }
        backpropagate(playout_node, simulation_result);
    }
    return get_most_simulated_child_square(root);
}