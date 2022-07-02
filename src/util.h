#ifndef UTTT2_UTIL_H
#define UTTT2_UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "square.h"


#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CHECK(a,b) ((a) & (1ULL<<(b)))

void* safe_malloc(size_t size);

void* safe_calloc(size_t size);

void safe_free(void* pointer);

void crash(char* errorMessage);

void assertMsg(bool condition, char* errorMessage);

Square toOurNotation(Square rowAndColumn);

#endif //UTTT2_UTIL_H
