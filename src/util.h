#ifndef UTTTCODINGAMEC_UTIL_H
#define UTTTCODINGAMEC_UTIL_H

#include <stdbool.h>
#include <stdint.h>

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

void* safe_malloc(size_t size);

void* safe_calloc(size_t size);

void my_assert(bool condition, char* msg);

bool check_bit(uint16_t value, int bit_index);

#endif //UTTTCODINGAMEC_UTIL_H
