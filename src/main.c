#include <stdio.h>
#include "../test/test_main.h"
#include "board.h"
#include "find_best_move.h"


void skip_moves_input() {
    int valid_action_count;
    scanf("%d", &valid_action_count);
    for (int i = 0; i < valid_action_count; i++) {
        int row;
        int col;
        scanf("%d%d", &row, &col);
    }
}


void print_move(MCTSNode* root, Square best_move) {
    Square s = to_game_notation(best_move);
    int8_t x = s.grid;
    int8_t y = s.pos;
    int sims = get_sims(root);
    double wins = get_wins(root);
    double eval = wins / sims;
    printf("%d %d EVAL: %.2f SIMS: %d\n", x, y, eval, sims);
}


#define TIME 5
int main() {
    run_tests();
    Board* board = init_board();
    MCTSNode* root = init_MCTS_root_node(PLAYER_2);
    pcg32_random_t rng;
    pcg32_srandom_r(&rng, 69, 420);
    bool first = true;
    while (true) {
        int enemy_row;
        int enemy_col;
        scanf("%d%d", &enemy_row, &enemy_col);
        skip_moves_input();
        if (enemy_row != -1) {
            Square enemy_move = to_our_notation(enemy_row, enemy_col);
            make_permanent_move(board, enemy_move);
            if (!first) {
                root = get_next_root(root, board, enemy_move);
            }
        }
        Square best_move = find_best_move(board, root, TIME, &rng);
        make_permanent_move(board, best_move);
        root = get_next_root(root, board, best_move);
        print_move(root, best_move);
        first = false;
    }
}