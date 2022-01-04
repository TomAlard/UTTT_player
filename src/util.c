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


void my_assert(bool condition, char* msg) {
    if (!condition) {
        fprintf(stderr, "Assertion failed: %s\n", msg);
        exit(1);
    }
}


clock_t get_deadline(double time) {
    return clock() + time*CLOCKS_PER_SEC;
}


bool has_time_remaining(clock_t deadline) {
    return clock() < deadline;
}


bool check_bit(uint16_t value, int bit_index) {
    return (value >> bit_index) & 1;
}
