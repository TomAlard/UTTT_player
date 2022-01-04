#include "square.h"


bool squares_are_equal(Square a, Square b) {
    return a.grid == b.grid && a.pos == b.pos;
}


Square to_our_notation(int row, int col) {
    int8_t grid = 3*(row / 3) + (col / 3);
    int8_t pos = 3*(row % 3) + (col % 3);
    Square result = {grid, pos};
    return result;
}


Square to_game_notation(Square square) {
    int8_t x = square.pos / 3;
    int8_t y = square.pos % 3;
    int8_t row = x + 3*(square.grid / 3);
    int8_t col = y + 3*(square.grid % 3);
    Square result = {row, col};
    return result;
}


Player other_player(Player player) {
    return 1 - player;
}