#ifndef UTTT2_SQUARE_H
#define UTTT2_SQUARE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Square {
    uint8_t board;
    uint8_t position;
} Square;

bool squaresAreEqual(Square square1, Square square2);

#endif //UTTT2_SQUARE_H
