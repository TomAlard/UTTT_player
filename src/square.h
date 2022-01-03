#ifndef UTTTCODINGAMEC_SQUARE_H
#define UTTTCODINGAMEC_SQUARE_H

#include <stdint.h>


typedef struct Square {
    int8_t grid;
    int8_t pos;
} Square;


typedef enum Player {
    PLAYER_1,
    PLAYER_2
} Player;


Player other_player(Player player);

#endif //UTTTCODINGAMEC_SQUARE_H
