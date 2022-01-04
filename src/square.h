#ifndef UTTTCODINGAMEC_SQUARE_H
#define UTTTCODINGAMEC_SQUARE_H

#include <stdint.h>
#include <stdbool.h>


typedef struct Square {
    int8_t grid;
    int8_t pos;
} Square;

bool squares_are_equal(Square a, Square b);

Square to_our_notation(int row, int col);

Square to_game_notation(Square square);

typedef enum Player {
    PLAYER_1,
    PLAYER_2
} Player;


Player other_player(Player player);

#endif //UTTTCODINGAMEC_SQUARE_H
