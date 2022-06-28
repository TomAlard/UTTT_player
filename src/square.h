#ifndef UTTT2_SQUARE_H
#define UTTT2_SQUARE_H

#include <stdint.h>

typedef struct Square {
    uint8_t board;
    uint8_t position;
} Square;

Square createSquare(uint8_t board, uint8_t position);

#endif //UTTT2_SQUARE_H
