#include <stdalign.h>
#include "lookup_table_indices.h"


alignas(64) uint16_t LookupTableIndices[256][8];

void computeLookupTable() {
    for (int i = 0; i < 256; i++) {
        int j = i;
        int k = 0;
        while (j) {
            int lsbIndex = __builtin_ffs(j) - 1;
            j &= j - 1;
            LookupTableIndices[i][k++] = lsbIndex;
        }
    }
}