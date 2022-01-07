#include <stdlib.h>
#include <stdio.h>
#include "test_board.h"
#include "../src/board.h"
#include "../src/util.h"


void test_init_board() {
    Board* board = init_board();
    my_assert(calculate_board_state(board) == UNDECIDED, "test_init_board: Invalid empty board state");
    Square moves[TOTAL_SQUARES];
    int amount_of_moves = get_possible_moves(board, moves);
    my_assert(amount_of_moves == TOTAL_SQUARES, "test_init_board: Invalid move generation for empty board");
    free_board(board);
}


void test_update_board() {
    Board* board = init_board();
    Square s = {4, 4};
    Square moves[TOTAL_SQUARES];

    make_temporary_move(board, s);
    char* fail_msg_1 = "test_update_board: Incorrect move generation after temporary move";
    my_assert(get_possible_moves(board, moves) == TOTAL_SQUARES_SMALL_BOARD - 1, fail_msg_1);

    undo_all_temporary_moves(board);
    char* fail_msg_2 = "test_update_board: Incorrect move generation after first board reset";
    my_assert(get_possible_moves(board, moves) == TOTAL_SQUARES, fail_msg_2);

    make_permanent_move(board, s);
    char* fail_msg_3 = "test_update_board: Incorrect move generation after permanent move";
    my_assert(get_possible_moves(board, moves) == TOTAL_SQUARES_SMALL_BOARD - 1, fail_msg_3);

    undo_all_temporary_moves(board);
    char* fail_msg_4 = "test_update_board: Incorrect move generation after second board reset";
    my_assert(get_possible_moves(board, moves) == TOTAL_SQUARES_SMALL_BOARD - 1, fail_msg_4);

    free_board(board);
}


void test_rollout() {
    Board* board = init_board();
    srand(69420);  // NOLINT(cert-msc51-cpp)
    Square moves[TOTAL_SQUARES];
    while (calculate_board_state(board) == UNDECIDED) {
        int amount_of_moves = get_possible_moves(board, moves);
        Square move = moves[rand() % amount_of_moves];  // NOLINT(cert-msc50-cpp)
        make_temporary_move(board, move);
    }
    free_board(board);
}


void run_board_tests() {
    fprintf(stderr, "test_init_board\n");
    test_init_board();
    fprintf(stderr, "test_update_board\n");
    test_update_board();
    fprintf(stderr, "test_rollout\n");
    test_rollout();
}