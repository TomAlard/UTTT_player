#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "util.h"


#define AMOUNT_OF_PLAYERS 2
#define TOTAL_SQUARES_BIG_BOARD TOTAL_SQUARES_SMALL_BOARD
#define TOTAL_POSITIONS_SMALL_BOARD 512  // 2^9
#define FULL 511  // 2^9 - 1

typedef uint16_t BitBoard;

typedef struct Board {
    BitBoard bit_boards[AMOUNT_OF_PLAYERS][TOTAL_SQUARES_BIG_BOARD];
    BitBoard big_board_bit_boards[AMOUNT_OF_PLAYERS + 1];
    int8_t next_small_board;
    Player player_to_move;
    BitBoard reset_bit_boards[AMOUNT_OF_PLAYERS][TOTAL_SQUARES_BIG_BOARD];
    BitBoard reset_big_board_bit_boards[AMOUNT_OF_PLAYERS + 1];
    Player reset_player_to_move;
    int8_t reset_next_small_board;

    Square precomputed_empty_squares[TOTAL_POSITIONS_SMALL_BOARD][TOTAL_SQUARES_BIG_BOARD][TOTAL_SQUARES_SMALL_BOARD];
    State precomputed_states[AMOUNT_OF_PLAYERS][TOTAL_POSITIONS_SMALL_BOARD];
} Board;


void fill_empty_squares(Board* board, BitBoard position) {
    BitBoard original_position = position;
    for (int8_t grid = 0; grid < TOTAL_SQUARES_BIG_BOARD; grid++) {
        BitBoard position_copy = position;
        int next = 0;
        for (int8_t index_in_grid = 0; index_in_grid < TOTAL_SQUARES_SMALL_BOARD; index_in_grid++) {
            if ((position_copy & 1) == 0) {
                Square s = {grid, index_in_grid};
                board->precomputed_empty_squares[original_position][grid][next++] = s;
            }
            position_copy >>= 1;
        }
        if (next < TOTAL_SQUARES_SMALL_BOARD) {
            Square invalid = {-1, -1};
            board->precomputed_empty_squares[original_position][grid][next] = invalid;
        }
    }
}


bool is_won(BitBoard position) {
    bool bits[TOTAL_SQUARES_SMALL_BOARD];
    for (int i = 0; i < TOTAL_SQUARES_SMALL_BOARD; i++) {
        bits[i] = position & 1;
        position >>= 1;
    }

    const int lines[8][3] = {
            // horizontal lines
            {0, 1, 2},
            {3, 4, 5},
            {6, 7, 8},

            // vertical lines
            {0, 3, 6},
            {1, 4, 7},
            {2, 5, 8},

            // diagonals
            {0, 4, 8},
            {2, 4, 6}
    };

    for (int i = 0; i < 8; i++) {
        if (bits[lines[i][0]] && bits[lines[i][1]] && bits[lines[i][2]]) {
            return true;
        }
    }
    return false;
}


Board* init_board() {
    Board* board = safe_calloc(sizeof(Board));  // calloc, not malloc! Bitboards are initialised to 0!
    board->player_to_move = PLAYER_1;
    board->reset_player_to_move = PLAYER_1;
    board->next_small_board = -1;
    board->reset_next_small_board = -1;
    for (BitBoard position = 0; position < FULL; position++) {
        fill_empty_squares(board, position);
        bool won = is_won(position);
        board->precomputed_states[PLAYER_1][position] = won? PLAYER_1_WIN : UNDECIDED;
        board->precomputed_states[PLAYER_2][position] = won? PLAYER_2_WIN : UNDECIDED;
    }
    board->precomputed_states[PLAYER_1][FULL] = DRAW;
    board->precomputed_states[PLAYER_2][FULL] = DRAW;
    return board;
}


void free_board(Board* board) {
    free(board);
}


void undo_all_temporary_moves(Board* board) {
    memcpy(board->bit_boards, board->reset_bit_boards,
           sizeof(BitBoard) * AMOUNT_OF_PLAYERS * TOTAL_SQUARES_BIG_BOARD);
    memcpy(board->big_board_bit_boards, board->reset_big_board_bit_boards,
           sizeof(BitBoard) * (AMOUNT_OF_PLAYERS + 1));
    board->player_to_move = board->reset_player_to_move;
    board->next_small_board = board->reset_next_small_board;
}


void make_temporary_move(Board* board, Square square) {
    board->bit_boards[board->player_to_move][square.grid] |= 1 << square.pos;
    board->next_small_board = square.pos;
    BitBoard new_position = board->bit_boards[board->player_to_move][square.grid];
    State new_state = board->precomputed_states[board->player_to_move][new_position];
    if (new_state != UNDECIDED) {
        int index = new_state == DRAW? 2 : board->player_to_move;
        board->big_board_bit_boards[index] |= 1 << square.grid;
    }
    board->player_to_move = other_player(board->player_to_move);
}


void make_permanent_move(Board* board, Square square) {
    make_temporary_move(board, square);
    board->reset_bit_boards[board->player_to_move][square.grid] |= 1 << square.pos;
    board->reset_next_small_board = square.pos;
    BitBoard new_position = board->bit_boards[board->player_to_move][square.grid];
    State new_state = board->precomputed_states[board->player_to_move][new_position];
    if (new_state != UNDECIDED) {
        int index = new_state == DRAW? 2 : board->player_to_move;
        board->reset_big_board_bit_boards[index] |= 1 << square.grid;
    }
    board->reset_player_to_move = other_player(board->player_to_move);
}


int popcount(BitBoard bit_board) {
    int c = 0;
    while (bit_board) {
        bit_board &= bit_board - 1; // clear the least significant bit set
        c++;
    }
    return c;
}


State get_board_state(Board* board) {
    Player potential_winner = other_player(board->player_to_move);
    State state = board->precomputed_states[potential_winner][board->big_board_bit_boards[potential_winner]];
    if (state != UNDECIDED) {
        return state;
    }

    BitBoard bit_board = board->big_board_bit_boards[0] | board->big_board_bit_boards[1] | board->big_board_bit_boards[2];
    if (bit_board == FULL) {
        int player_1_small_boards = popcount(board->big_board_bit_boards[PLAYER_1]);
        int player_2_small_boards = popcount(board->big_board_bit_boards[PLAYER_2]);
        if (player_1_small_boards > player_2_small_boards) {
            return PLAYER_1_WIN;
        } if (player_1_small_boards < player_2_small_boards) {
            return PLAYER_2_WIN;
        }
        return DRAW;
    }
    return UNDECIDED;
}


int copy_empty_moves(Board* board, int8_t grid, Square* moves, int copy_start) {
    BitBoard small_board_set_bits = board->bit_boards[PLAYER_1][grid] | board->bit_boards[PLAYER_2][grid];
    Square* empty_squares = board->precomputed_empty_squares[small_board_set_bits][grid];
    for (int i = 0; i < TOTAL_SQUARES_SMALL_BOARD && empty_squares[i].grid != -1; i++) {
        moves[copy_start] = empty_squares[i];
        copy_start++;
    }
    return copy_start;
}


int get_possible_moves(Board* board, Square* moves) {
    BitBoard bit_board = board->big_board_bit_boards[0] | board->big_board_bit_boards[1] | board->big_board_bit_boards[2];
    if (board->next_small_board != -1 && !check_bit(bit_board, board->next_small_board)) {
        return copy_empty_moves(board, board->next_small_board, moves, 0);
    }

    int copy_start = 0;
    for (int8_t grid = 0; grid < TOTAL_SQUARES_BIG_BOARD; grid++) {
        bool grid_is_decided = check_bit(bit_board, grid);
        if (!grid_is_decided) {
            copy_start = copy_empty_moves(board, grid, moves, copy_start);
        }
    }
    return copy_start;
}
