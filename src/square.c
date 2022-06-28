#include "square.h"
#include "util.h"


Square createSquare(uint8_t board, uint8_t position) {
    if (board > 8 || position > 8) {
        crash("Square board and position must be between 0 and 8 inclusive");
    }
    Square result = {board, position};
    return result;
}
