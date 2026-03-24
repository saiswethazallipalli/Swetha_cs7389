#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WORK 1
#define REQUEST 2
#define ACK 3
#define STOP 4
#define COUNT 5

int busy_work(int x) {
    volatile int val = x;
    for (int i = 0; i < 2000; i++) {
        val = (val * 7 + 3) % 100000;
    }
    return val;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int runtime = atoi(argv[1]);
    double start = MPI_Wtime();

    int producers = (size - 1) / 2;
    int consumers = (size - 1) - producers;

    int local_count = 0;

    if (rank == 0) {
        int total = 0;
        int finished = 0;

        while (finished < size - 1) {
            int data;
            MPI_Status status;

            MPI_Recv(&data, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == COUNT) {
                total += data;
                finished++;
            } else if (status.MPI_TAG == WORK) {
                MPI_Send(&data, 1, MPI_INT, status.MPI_SOURCE, ACK, MPI_COMM_WORLD);
            } else if (status.MPI_TAG == REQUEST) {
                if ((MPI_Wtime() - start) < runtime) {
                    int work = rand() % 1000;
                    MPI_Send(&work, 1, MPI_INT, status.MPI_SOURCE, WORK, MPI_COMM_WORLD);
                } else {
                    MPI_Send(&data, 1, MPI_INT, status.MPI_SOURCE, STOP, MPI_COMM_WORLD);
                }
            }
        }

        printf("Total number of messages consumed: %d\n", total);

    } else if (rank <= producers) {
        while ((MPI_Wtime() - start) < runtime) {
            int value = rand() % 1000;
            MPI_Request req;

            MPI_Isend(&value, 1, MPI_INT, 0, WORK, MPI_COMM_WORLD, &req);
            busy_work(value);
            MPI_Wait(&req, MPI_STATUS_IGNORE);

            int ack;
            MPI_Recv(&ack, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        MPI_Send(&local_count, 1, MPI_INT, 0, COUNT, MPI_COMM_WORLD);

    } else {
        while (1) {
            int req = 1;
            MPI_Send(&req, 1, MPI_INT, 0, REQUEST, MPI_COMM_WORLD);

            int msg;
            MPI_Status status;
            MPI_Recv(&msg, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == WORK) {
                local_count++;
                busy_work(msg);
            } else if (status.MPI_TAG == STOP) {
                break;
            }
        }

        MPI_Send(&local_count, 1, MPI_INT, 0, COUNT, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
