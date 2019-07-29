#include <stdio.h>
#include <city.h>
#include <assert.h>
#include "HashTable.h"

#define MAX 12
int main() {

    HashTable<1> table;

    for (uint64_t i = 0; i < MAX; ++i) {
        uint8_t *k = (uint8_t *)&i;
        uint64_t vv = i * 2;
        uint8_t *v = (uint8_t *)&vv;

        table.put(k, 8, v, 8);
    }

    for (uint64_t i = 0; i < MAX; ++i) {
        // if (i % 2 == 0) {
            uint8_t *k = (uint8_t *)&i;
            table.del(k, 8);
        // }
    }

    for (uint64_t i = 0; i < MAX; ++i) {
        uint8_t *k = (uint8_t *)&i;
        uint16_t l;
        uint8_t *v = table.get(k, 8, &l);

        // if (i % 2 == 0) {
            assert(v == NULL);
        // } else {
            // assert(l == 8 && *(uint64_t *)v == i * 2);
        // }
    }
    // assert(table.table[0] == NULL);
    return 0;
}
