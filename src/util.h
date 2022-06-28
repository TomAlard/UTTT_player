#ifndef UTTT2_UTIL_H
#define UTTT2_UTIL_H

#include <stdbool.h>
#include <stdint.h>


#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) ((a) & (1ULL<<(b)))

void* safe_malloc(size_t size);

void* safe_calloc(size_t size);

void safe_free(void* pointer);

void crash(char* errorMessage);

#endif //UTTT2_UTIL_H
