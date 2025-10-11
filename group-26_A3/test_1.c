// File: test_1.c
#include <stdio.h>
#include "dummy_main.h"

int main(int argc, char **argv) {
    // Busy computation without blocking calls
    int sum = 0;
    printf("Program1 (PID: %d): Starting computation.\n", getpid());
    fflush(stdout);

    for (int i = 0; i < 20; i++) {
        sum += i;
        if (i > 0 && i % 5 == 0) {
            printf("Program1: Progress...\n");
            fflush(stdout);
        }
    }
    printf("Program1: Final sum = %d\n", sum);
    fflush(stdout);
    return 0;
}
