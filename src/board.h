#ifndef UTTTCODINGAMEC_BOARD_H
#define UTTTCODINGAMEC_BOARD_H

#include "square.h"

#define TOTAL_SQUARES 81
#define TOTAL_SQUARES_SMALL_BOARD 9

typedef enum State {
    UNDECIDED,
    DRAW,
    PLAYER_1_WIN,
    PLAYER_2_WIN
} State;

typedef struct Board Board;

Board* init_board();

void free_board(Board* board);

void undo_all_temporary_moves(Board* board);

void make_temporary_move(Board* board, Square square);

void make_permanent_move(Board* board, Square square);

State calculate_board_state(Board* board);

int get_possible_moves(Board* board, Square* moves);

#endif //UTTTCODINGAMEC_BOARD_H
