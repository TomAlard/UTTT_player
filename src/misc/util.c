#include <stdlib.h>
#include "util.h"


void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Couldn't allocate %zu bytes of memory!\n", size);
        exit(1);
    }
    return ptr;
}


void* safe_calloc(size_t size) {
    void* ptr = calloc(1, size);
    if (ptr == NULL) {
        fprintf(stderr, "Couldn't allocate %zu bytes of memory!\n", size);
        exit(1);
    }
    return ptr;
}


void* safe_realloc(void* pointer, size_t size) {
    void* ptr = realloc(pointer, size);
    if (ptr == NULL) {
        fprintf(stderr, "Couldn't reallocate %zu bytes of memory!\n", size);
        exit(1);
    }
    return ptr;
}


void safe_free(void* pointer) {
    if (pointer == NULL) {
        fprintf(stderr, "Cannot free NULL pointer!\n");
        exit(1);
    }
    free(pointer);
}


Square toOurNotation(Square rowAndColumn) {
    uint8_t row = rowAndColumn.board;
    uint8_t column = rowAndColumn.position;
    if (row == 255 && column == 255) {
        Square result = {9, 9};
        return result;
    }
    uint8_t board = 3 * (row / 3) + (column / 3);
    uint8_t position = 3 * (row % 3) + (column % 3);
    Square result = {board, position};
    return result;
}


Square toGameNotation(Square square) {
    uint8_t row = (square.position / 3) + 3*(square.board / 3);
    uint8_t col = (square.position % 3) + 3*(square.board % 3);
    Square result = {row, col};
    return result;
}


void writePositionToFile(State* state, FILE* file, float winrate, Square bestMove) {
    char p1SmallBoardBits[81];
    char p2SmallBoardBits[81];
    for (int j = 0; j < 81; j++) {
        p1SmallBoardBits[j] = (char)((BIT_CHECK(state->player1.smallBoards[j/9], j%9) != 0) + '0');
        p2SmallBoardBits[j] = (char)((BIT_CHECK(state->player2.smallBoards[j/9], j%9) != 0) + '0');
    }
    char p1BigBoardBits[9];
    char p2BigBoardBits[9];
    for (int j = 0; j < 9; j++) {
        p1BigBoardBits[j] = (char)((BIT_CHECK(state->player1.bigBoard, j) != 0) + '0');
        p2BigBoardBits[j] = (char)((BIT_CHECK(state->player2.bigBoard, j) != 0) + '0');
    }
    fprintf(file, "%.*s,%.*s,%.*s,%.*s,%d,%d,%.4f,%d%d\n", 81, p1SmallBoardBits, 81, p2SmallBoardBits,
            9, p1BigBoardBits, 9, p2BigBoardBits, state->currentPlayer, state->currentBoard, winrate, bestMove.board, bestMove.position);
}
