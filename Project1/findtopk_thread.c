#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "common.h"

#define MAXK_SIZE 1000

void* runner(void* args);
int* findtopk(int* arr, int k, int N, char* read_filename);

struct Args {
    int k;
    int N;
    int id;
    char* read_filename;
    int* arr;
};

int main(int argc, char* argv[]) {
    int k = atoi(argv[1]);
    int N = atoi(argv[2]);

    if (argc != N + 4) {
        printf("Wrong number of arguments.\nUsage: findtopk <k> <N> <infile1>...<infileN> <outfile>\n");
        exit(1);
    }
    printf("k: %d, N: %d\n", k, N);

    char* out_filename = argv[3 + N];
    printf("out_filename: %s\n", out_filename);

    int* arrays[MAX_N_SIZE];
    for (int i = 0; i < N; ++i) {
        arrays[i] = malloc(sizeof(int) * k);
        for (int j = 0; j < k; ++j)
            arrays[i][j] = 0;
    }
    pthread_t tids[MAX_N_SIZE];
    pthread_attr_t attr[MAX_N_SIZE];
    for (int i = 0; i < N; ++i)
        pthread_attr_init(&attr[i]);

    for(int i = 0; i < N; ++i) {
        char* read_filename = argv[i + 3];

        struct Args* args = malloc(sizeof(struct Args));
        args->k = k;
        args->N = N;
        args->id = i;
        args->read_filename = read_filename;
        args->arr = arrays[i];

        pthread_create(&tids[i], &attr[i], runner, args);
    }

    void* retval;
    for (int i = 0; i < N; ++i) {
        pthread_join(tids[i], &retval);
        arrays[i] = (int*) retval;
        // printf("Thread %d returned: \n", i);
        /* for (int j = 0; j < k; ++j) {
            printf("%d ", arrays[i][j]);
        } */
        // printf("\n");
    }

    int* outArray = malloc(sizeof(int) * k * N);
    for (int i = 0; i < k * N; ++i)
        outArray[i] = 0;
    merge(arrays, N, k, outArray);

    int out_fd = open(out_filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    for (int i = 0; i < k * N; ++i) {
        char buf[20];
        sprintf(buf, "%d\n", outArray[i]);
        write(out_fd, buf, strlen(buf));
    }
    close(out_fd);
    // printf("Success\n");
}

void* runner(void* a) {
    struct Args* args = (struct Args*) a;

    // printf("Thread with id %d:\nk: %d, N: %d\nfilename: %s\n", args->id, args->k, args->N, args->read_filename);

    void* maxk = findtopk(args->arr, args->k, args->N, args->read_filename);

    pthread_exit(maxk);
    return maxk;
}

int* findtopk(int* maxk, int k, int N, char* read_filename) {
    // printf("Find maxk %s:\n", read_filename);
    // int* maxk = malloc(sizeof(int) * k);
    int read_fd = open(read_filename, O_RDONLY);
    for (int i = 0; i < k; ++i)
        maxk[i] = 0;
    // printf("\n");
    findmaxk(read_fd, maxk, k);

    close(read_fd);
    return maxk;
}