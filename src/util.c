#include <stdlib.h>
#include <stdio.h>
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


void safe_free(void* pointer) {
    if (pointer == NULL) {
        fprintf(stderr, "Cannot free NULL pointer!\n");
        exit(1);
    }
    free(pointer);
}


void crash(char* errorMessage) {
    fprintf(stderr, "%s\n", errorMessage);
    exit(1);
}


void assertMsg(bool condition, char* errorMessage) {
    if (!condition) {
        crash(errorMessage);
    }
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
