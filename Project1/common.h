#ifndef PROJECT1_UTIL_H__
#define PROJECT1_UTIL_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_N_SIZE 5
#define STRINGFY(n) #n
#ifndef TEMP
#define TEMP(n) "tmp" STRINGFY(n)
#endif
#define TEMP_DEF "tmp0"

int readint(int fd);

void findmaxk(int read_fd, int* maxk, int k)
{
    int current;

    while (1) {
        current = readint(read_fd);
        if (current < 0)
            return;
        // printf("integer: %d\n", current);

        int i = 0;
        while (i < k && current > maxk[i])
            i += 1;

        i -= 1;
        // Shift left previous
        for (int j = 0; j < i; ++j) {
            maxk[j] = maxk[j + 1];
        }
        maxk[i] = current;
    }
}

int readint(int fd) 
{
    char buffer[20];
    int i = 0;
    while (1) {
        size_t s = read(fd, &buffer[i], 1);
        switch (s)
        {
            case -1:
                return -1;
            case 0:
                if (i == 0) {
                    return -1;
                } else {
                    buffer[i] = '\0';
                    return atoi(buffer);
                }
            case 1:
                if (buffer[i] == ' ' || buffer[i] == EOF || buffer[i] == '\n') {
                    buffer[i] = '\0';
                    return atoi(buffer);
                }
                i += 1;
                break;
            default:
                return -1;
        }
    } 
}

int finished_merge(int* cursors, int N, int k);

/**
 * maxNums: int[N][k]
 * outArray: int[N * k] should be allocated previously
 */
void merge(int** maxNums, int N, int k, int* outArray) {
    /* printf("Implement: merging arrays...\n");
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < k; ++j) {
            outArray[(i * k) + j] = maxNums[i][j];
        }
    } */
    int cursors[MAX_N_SIZE];

    for (int i = 0; i < N; ++i)
        cursors[i] = 0;

    for (int out_cursor = 0; out_cursor < N * k; out_cursor++) {
        // TODO
        int min = INT_MAX, minIndex = -1;
        for (int i = 0; i < N; ++i) {
            if (cursors[i] < k) {
                int num = maxNums[i][cursors[i]];
                // printf("Number is: %d\n", num);
                if (num < min) {
                    min = num;
                    minIndex = i;
                }
            }
        }
        // printf("min value: %d\n", min);
        outArray[out_cursor] = min;
        cursors[minIndex] += 1;
    }    
}

int finished_merge(int* cursors, int N, int k) {
    int finished = 0;
    for (int i = 0; i < N; ++i)
        finished = finished && cursors[i];

    return finished;
}

#endif