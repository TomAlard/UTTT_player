#include <stdio.h>
#include <stdlib.h>
#include "test_util.h"


void myAssert(bool condition) {
    if (!condition) {
        fprintf(stderr, "Assertion failed\n");
        exit(1);
    }
}