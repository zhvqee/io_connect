#include <stdio.h>

#include "nio/callback.h"

void *testAccept(void *arg) {
    printf("current socketId %d", (int *) arg);
}

int main() {
    afterAccept(testAccept, 4);
    afterAccept(NULL,5);
}
