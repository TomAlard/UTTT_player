#include "main.h"
#include "../test/test_main.h"
#include "find_best_move.h"


void skip_moves_input(FILE* file) {
    int valid_action_count;
    fscanf(file, "%d", &valid_action_count);
    for (int i = 0; i < valid_action_count; i++) {
        int row;
        int col;
        fscanf(file, "%d%d", &row, &col);
    }
}


void print_move(MCTSNode* root, Square best_move) {
    Square s = to_game_notation(best_move);
    int8_t x = s.grid;
    int8_t y = s.pos;
    int sims = get_sims(root);
    double wins = get_wins(root);
    printf("%d %d %.1f/%d\n", x, y, wins, sims);
    fflush(stdout);
}


Square play_best_move(int enemy_row, int enemy_col, Board* board, MCTSNode** root, pcg32_random_t* rng, double time,
                      bool first) {
    if (enemy_row != -1) {
        Square enemy_move = to_our_notation(enemy_row, enemy_col);
        make_permanent_move(board, enemy_move);
        if (!first) {
            *root = get_next_root(*root, board, enemy_move);
        }
    }
    Square best_move = find_best_move(board, *root, time, rng);
    make_permanent_move(board, best_move);
    return best_move;
}


void play_game(FILE* file, double time) {
    Board* board = init_board();
    MCTSNode* root = init_MCTS_root_node(PLAYER_2);
    pcg32_random_t rng;
    pcg32_srandom_r(&rng, 69, 420);
    bool first = true;
    while (true) {
        int enemy_row;
        int enemy_col;
        int v = fscanf(file, "%d%d", &enemy_row, &enemy_col);
        if (v == EOF) {
            break;
        }
        skip_moves_input(file);
        Square best_move = play_best_move(enemy_row, enemy_col, board, &root, &rng, time, first);
        fprintf(stderr, "TOTAL SIMS: %d\n", get_sims(root));
        root = get_next_root(root, board, best_move);
        print_move(root, best_move);
        first = false;
    }
    free_board(board);
    free_MCTS_tree(root);
}


#define TIME 0.0999
int main() {
    run_tests();
    play_game(stdin, TIME);
}