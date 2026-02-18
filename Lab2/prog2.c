#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int nproc, rank;
    int value = -1;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int next = (rank + 1) % nproc;
    int prev = (rank - 1 + nproc) % nproc;

    if (rank == 0) {
        srand((unsigned)time(NULL));
        value = rand() % 1000;  // random number
        printf("Rank 0 initial value = %d\n", value);

        // send to rank 1
        MPI_Send(&value, 1, MPI_INT, next, 0, MPI_COMM_WORLD);

        // receive from last rank
        MPI_Recv(&value, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank 0 received back value = %d from rank %d. Done.\n", value, prev);
    } else {
        // receive from previous rank
        MPI_Recv(&value, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank %d received value %d from rank %d\n", rank, value, prev);

        // send to next rank
        MPI_Send(&value, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
        printf("Rank %d sent value %d to rank %d\n", rank, value, next);
    }

    MPI_Finalize();
    return 0;
}

