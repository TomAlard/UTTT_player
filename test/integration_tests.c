#include <stdio.h>
#include "integration_tests.h"
#include "../src/board.h"
#include "../src/mcts_node.h"
#include "../src/random.h"
#include "../src/find_best_move.h"


void play_against_self() {
    Board* board = init_board();
    MCTSNode* root = init_MCTS_root_node(PLAYER_2);
    pcg32_random_t rng;
    pcg32_srandom_r(&rng, 69, 420);
    double time = 0.1;
    while (calculate_board_state(board) == UNDECIDED) {
        Square best_move = find_best_move(board, root, time, &rng);
        make_permanent_move(board, best_move);
        root = get_next_root(root, board, best_move);
    }
    free_MCTS_tree(root);
    free_board(board);
}


void run_integration_tests() {
    fprintf(stderr, "play_against_self\n");
    play_against_self();
}