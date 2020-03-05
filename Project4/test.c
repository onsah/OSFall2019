#include <assert.h>
#include <stdio.h>
#include "simplefs.h"

int main() {
    // 2^21 bytes
    assert(sizeof(Block) == BLOCK_SIZE);

    char *text = "Hello world";

    

    printf("Test succeeded :)\n");
}