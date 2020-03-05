#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <wait.h>
#include <unistd.h>
#include <string.h>

#define TEMP(n) { 't', 'm', 'p', (n) + '0', '\0' } 
#define MAX_N 5

#include "common.h"

void findtopk(int k, int N, char* read_filename, char* out_filename);

int main(int argc, char* argv[])
{
    int k = atoi(argv[1]);
    int N = atoi(argv[2]);

    if (argc != N + 4) {
        printf("Wrong number of arguments.\nUsage: findtopk <k> <N> <infile1>...<infileN> <outfile>\n");
        exit(1);
    }
    printf("k: %d, N: %d\n", k, N);

    char* out_filename = argv[3 + N];
    printf("out_filename: %s\n", out_filename);

    // subprocesses
    for (int i = 0; i < N; ++i) {
        char* read_filename = argv[i + 3];
        char out_filename[] = TEMP(i);
        
        // printf("Read file: %s, Write file: %s\n", read_filename, out_filename);
        findtopk(k, N, read_filename, out_filename);
    }

    // Wait all to terminate
    for (int i = 0; i < N; ++i)
        wait(0);

    // maxk nums for each file
    int** maxNums = malloc(sizeof(int*) * N);
    for (int i = 0; i < N; ++i)
        maxNums[i] = malloc(sizeof(int) * k);
    // Output file
    int out_fd = open(out_filename, O_CREAT | O_RDWR | O_TRUNC, 0644);

    // Read subfiles to array
    for (int i = 0; i < N; ++i) {
        char temp_name[] = TEMP(i);
        // printf("Reading file %s...\n", temp_name);
        int temp_fd = open(temp_name, O_RDONLY);

        for (int j = 0; j < k; ++j) {
            int current;
            // Read until success
            while (1) {
                current = readint(temp_fd);
                if (current > 0)
                    break;
            }
            
            // printf("Read: %d\n", current);
            maxNums[i][j] = current;
        }
        close(temp_fd);
        // We should cleanup our files
        remove(temp_name);
    }
    for (int i = 0; i < N; ++i) {
        // printf("File %d:\n", i);
        for (int j = 0; j < k; ++j) {
            // printf("%d, ", maxNums[i][j]);
        }
        // printf("\n");
    }
    
    int* outArray = malloc(sizeof(int) * k * N);
    for (int i = 0; i < k * N; ++i)
        outArray[i] = 0;
    merge(maxNums, N, k, outArray);

    for (int i = 0; i < k * N; ++i) {
        char buf[20];
        sprintf(buf, "%d\n", outArray[i]);
        write(out_fd, buf, strlen(buf));
    }
    close(out_fd);
    printf("Success\n");
}

void findtopk(int k, int N, char* read_filename, char* out_filename)
{
    pid_t p = fork();
    if (p == 0) {
        // printf("Opening files %s and %s\n", read_filename, out_filename);
        int read_fd = open(read_filename, O_RDONLY);
        int write_fd = open(out_filename, O_CREAT | O_WRONLY, 0644);

        int* maxk = malloc(sizeof(int) * k);
        for (int i = 0; i < k; ++i)
            maxk[i] = - 1;

        // Find max k numbers
        // printf("Finding max %d numbers...\n", k);
        findmaxk(read_fd, maxk, k);
        /* for (int i = 0; i < k; ++i)
            printf("%d, ", maxk[i]);
        printf("\n"); */

        // write them to file
        // printf("Writing to file %s...\n", out_filename);
        for (int i = 0; i < k; ++i) {
            char buffer[20];
            // itoa(maxk[i], buffer, 10);
            sprintf(buffer, "%d\n", maxk[i]);
            // printf("Writing the text %s", buffer);
            /* size_t s = */ write(write_fd, buffer, strlen(buffer));
            // printf("Success: %ld\n", s);
        }

        // Close resources
        // printf("Freeing resources...\n");
        // free(maxk);
        // printf("Closing files...\n");
        close(read_fd);
        close(write_fd);

        exit(0);
    }
}