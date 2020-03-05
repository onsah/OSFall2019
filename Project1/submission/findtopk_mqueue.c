#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <wait.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>

#include "common.h"

#define QUEUE_NAME "/mq_maxk"
#define MAXK_SIZE 1000
#define ARRAY_SIZE (sizeof(int) * MAXK_SIZE)

struct MaxK {
    int child_id;
    int maxk[MAXK_SIZE];
};

void findtopk(int k, int N, char* read_filename, int id);
mqd_t create_mq();

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

    // Open message queue
    // mqd_t mqd = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0600, 0);
    mqd_t mqd = create_mq();
    for (int i = 0; i < N; ++i) {
        char* read_filename = argv[i + 3];
        // printf("Read file: %s\n", read_filename);

        findtopk(k, N, read_filename, i);
    }

    for (int i = 0; i < N; ++i)
        wait(0);

    // mq_close(mqd);

    int out_fd = open(out_filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    unsigned int prio;

    int* arrays[MAX_N_SIZE];
    for (int i = 0; i < N; ++i) {
        arrays[i] = malloc(sizeof(int) * k);
        for (int j = 0; j < k; ++j)
            arrays[i][j] = 0;
    }

    for (int i = 0; i < N; ++i) {
        // printf("loop %d\n", i);
        struct MaxK maxKs[2];
        if (mq_receive(mqd, (char*) &maxKs[0], sizeof(struct MaxK) * 2, &prio) != -1) {
            // printf("Got message\n");
            char buff[20];
            for (int j = 0; j < k; ++j) {
                // printf("Number: %d\n", maxKs[0].maxk[j]);
                sprintf(buff, "%d\n", maxKs[0].maxk[j]);
                // write(out_fd, buff, strlen(buff));
                arrays[i][j] = maxKs[0].maxk[j];
            }
        } else {
            perror("mq_receive");
            exit(1);
        }   
    }

    int* outArray = malloc(sizeof(int) * k * N);
    for (int i = 0; i < k * N; ++i)
        outArray[i] = 0;
    merge(arrays, N, k, outArray);

    // int out_fd = open(out_filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    for (int i = 0; i < k * N; ++i) {
        char buf[20];
        sprintf(buf, "%d\n", outArray[i]);
        write(out_fd, buf, strlen(buf));
    }
    close(out_fd);
    mq_close(mqd);
    mq_unlink(QUEUE_NAME);
}

void findtopk(int k, int N, char* read_filename, int id)
{
    pid_t p = fork();
    if (p == 0) {
        // printf("Opening file %s\n", read_filename);
        int read_fd = open(read_filename, O_RDONLY);
        mqd_t mqd = mq_open(QUEUE_NAME, O_WRONLY);
        if (mqd == -1) {
            perror("mq_open");
            exit(1);
        }

        struct MaxK maxk;
        maxk.child_id = id;
        for (int i = 0; i < k; ++i)
            maxk.maxk[i] = -1;

        // Find max k numbers
        // printf("Finding max %d numbers...\n", k);
        findmaxk(read_fd, maxk.maxk, k);
        // printf("Max %d numbers: ", k);
        /* for (int i = 0; i < k; ++i)
            printf("%d, ", maxk.maxk[i]); */
        // printf("\n");

        // send from message queue
        // printf("Sending to queue...\n");
        if(mq_send(mqd, (const char*) &maxk, sizeof(struct MaxK), 1) == -1) {
            perror("mq_send");
            exit(1);
        } else {
            // printf("Sending success\n");
        }
        
        // Close resources
        // printf("Freeing resources...\n");
        // free(maxk);
        // printf("Closing files...\n");
        close(read_fd);
        close(mqd);

        exit(0);
    }
}

mqd_t create_mq() {
    struct mq_attr attr;  
    attr.mq_flags = 0;  
    attr.mq_maxmsg = 10;  
    attr.mq_msgsize = sizeof(struct MaxK) + 1;  
    attr.mq_curmsgs = 0;  

    mqd_t queue = mq_open(QUEUE_NAME, O_CREAT|O_RDWR, 0644, &attr);
    if (queue == -1) {
        perror("mq_open");
        exit(1);
    }
    // printf("Queue opened in: %d\n", queue);
    mq_getattr(queue, &attr);
    // printf("current messages: %ld\n", attr.mq_curmsgs);

    return queue;
}