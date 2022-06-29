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