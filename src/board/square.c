#include <assert.h>
#include "square.h"


Square createSquare(uint8_t board, uint8_t position) {
    assert(board <= 8 && position <= 8 && "Square board and position must be between 0 and 8 inclusive");
    Square result = {board, position};
    return result;
}


bool squaresAreEqual(Square square1, Square square2) {
    return square1.board == square2.board && square1.position == square2.position;
}
