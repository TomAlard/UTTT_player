#ifndef UTTT2_UTIL_H
#define UTTT2_UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "../board/square.h"


#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CHECK(a,b) ((a) & (1ULL<<(b)))
#define BIT_CHANGE(a,b,c) ((a) = (a) & ~(b) | (-(c) & (b)))

void* safe_malloc(size_t size);

void* safe_calloc(size_t size);

void* safe_realloc(void* pointer, size_t size);

void safe_free(void* pointer);

Square toOurNotation(Square rowAndColumn);

Square toGameNotation(Square square);

#endif //UTTT2_UTIL_H
