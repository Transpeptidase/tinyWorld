#include "../Lib/Common.h"
#include "../Lib/nvm.h"

#define _GB_ (1024ull * 1024 * 1024)

int io_size = 64;
uint64_t counter = 0;

void threader(char *m, size_t size) {

    while (true) {
        for (size_t offset = 0; offset < size; offset += io_size) {

            memset(m + offset, 0, io_size);
            // persistent::clflush_range(m + offset, io_size);

            counter++;
        }
    }
}


int main() {

    bindCore(0);
    ThroughPutCounter(&counter, 2);

    auto dram = persistent::alloc_nvm(50 * _GB_);
    auto nvm = persistent::alloc_nvm(250 * _GB_, "/dev/dax2.0");


    threader(dram, _GB_);

}


