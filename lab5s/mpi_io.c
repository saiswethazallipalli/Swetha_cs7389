/* noncontiguous access with a single collective I/O function */
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define FILESIZE 1024
#define INTS_PER_BLK 1

void fill_buffer(int *arr, int count, int rank) {
    for (int i = 0; i < count; i++) {
        arr[i] = rank * 100 + i;
    }
}

void print_sample(int *arr, int count, int rank) {
    int limit = (count < 8) ? count : 8;
    printf("Rank %d read: ", rank);
    for (int i = 0; i < limit; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    int rank, nprocs;
    int bufsize, nints, blocks;
    int *writebuf, *verifybuf;
    MPI_File fh;
    MPI_Datatype strided_type;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    bufsize = FILESIZE / nprocs;
    nints = bufsize / sizeof(int);
    blocks = nints / INTS_PER_BLK;

    writebuf = (int *)malloc(nints * sizeof(int));
    verifybuf = (int *)malloc(nints * sizeof(int));

    if (writebuf == NULL || verifybuf == NULL) {
        fprintf(stderr, "Rank %d could not allocate memory\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    fill_buffer(writebuf, nints, rank);

    for (int i = 0; i < nints; i++) {
        verifybuf[i] = 0;
    }

    MPI_Type_vector(blocks, INTS_PER_BLK, nprocs, MPI_INT, &strided_type);
    MPI_Type_commit(&strided_type);

    MPI_File_open(MPI_COMM_WORLD,
                  "lab5_collective.dat",
                  MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL,
                  &fh);

    MPI_File_set_view(fh,
                      rank * sizeof(int),
                      MPI_INT,
                      strided_type,
                      "native",
                      MPI_INFO_NULL);

    MPI_File_write_all(fh, writebuf, nints, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&fh);

    MPI_File_open(MPI_COMM_WORLD,
                  "lab5_collective.dat",
                  MPI_MODE_RDONLY,
                  MPI_INFO_NULL,
                  &fh);

    MPI_File_set_view(fh,
                      rank * sizeof(int),
                      MPI_INT,
                      strided_type,
                      "native",
                      MPI_INFO_NULL);

    MPI_File_read_all(fh, verifybuf, nints, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&fh);

    print_sample(verifybuf, nints, rank);

    MPI_Type_free(&strided_type);
    free(writebuf);
    free(verifybuf);

    MPI_Finalize();
    return 0;
}
