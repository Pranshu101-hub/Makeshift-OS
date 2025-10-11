// File: test_2.c
#include <stdio.h>
#include <unistd.h> // For getpid()
#include "dummy_main.h"

#define SIZE 400

int main(int argc, char **argv) {
    printf("MatrixMult (PID: %d): Allocating matrices...\n", getpid());
    fflush(stdout);
    
    static int A[SIZE][SIZE];
    static int B[SIZE][SIZE];
    static long long C[SIZE][SIZE];
    
    // Initialize matrices
    printf("MatrixMult: Initializing...\n");
    fflush(stdout);
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            A[i][j] = i + j;
            B[i][j] = i - j;
            C[i][j] = 0;
        }
    }
    
    // Matrix multiplication
    printf("MatrixMult: Computing...\n");
    fflush(stdout);
    for (int i = 0; i < SIZE; i++) {
        if (i > 0 && i % 100 == 0) {
            printf("MatrixMult: Row %d/%d\n", i, SIZE);
            fflush(stdout);
        }
        for (int j = 0; j < SIZE; j++) {
            for (int k = 0; k < SIZE; k++) {
                C[i][j] += (long long)A[i][k] * B[k][j];
            }
        }
    }
    
    printf("MatrixMult: Done! C[0][0] = %lld\n", C[0][0]);
    fflush(stdout);
    return 0;
}
