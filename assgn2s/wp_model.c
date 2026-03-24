#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TASK 1
#define CONFIRM 2

void simulate(int x) {
    volatile int v = x;
    for (int i = 0; i < 1500; i++) {
        v = (v * 5 + 1) % 100000;
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s <seconds>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    int duration = atoi(argv[1]);
    if (duration <= 0) {
        if (rank == 0) {
            fprintf(stderr, "Time must be > 0\n");
        }
        MPI_Finalize();
        return 1;
    }

    srand((unsigned int)(time(NULL) + rank * 100));
    double start = MPI_Wtime();
    int consumed = 0;

    while ((MPI_Wtime() - start) < duration) {
        int task = rand() % 1000;
        int target = rand() % size;

        MPI_Request req;
        MPI_Isend(&task, 1, MPI_INT, target, TASK, MPI_COMM_WORLD, &req);

        int ack_received = 0;

        while (!ack_received && (MPI_Wtime() - start) < duration) {
            int flag = 0;
            MPI_Status status;

            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

            if (flag) {
                int msg;
                MPI_Recv(&msg, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG,
                         MPI_COMM_WORLD, &status);

                if (status.MPI_TAG == TASK) {
                    int ack = 1;
                    MPI_Send(&ack, 1, MPI_INT, status.MPI_SOURCE, CONFIRM, MPI_COMM_WORLD);
                    consumed++;
                    simulate(msg);
                } else if (status.MPI_TAG == CONFIRM && status.MPI_SOURCE == target) {
                    ack_received = 1;
                }
            } else {
                simulate(task);
            }
        }

        MPI_Wait(&req, MPI_STATUS_IGNORE);
    }

    int total = 0;
    MPI_Reduce(&consumed, &total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Total number of messages consumed: %d\n", total);
    }

    MPI_Finalize();
    return 0;
}
