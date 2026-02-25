#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

static float* make_random(int n) {
    float *a = (float*)malloc((size_t)n * sizeof(float));
    if (!a) { perror("malloc"); MPI_Abort(MPI_COMM_WORLD, 1); }
    for (int i = 0; i < n; i++)
        a[i] = (float)rand() / (float)RAND_MAX;
    return a;
}

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    int rank, nproc;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (argc != 2) {
        if (rank == 0)
            fprintf(stderr, "Usage: %s N\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int N = atoi(argv[1]);
    if (N <= 0) {
        if (rank == 0)
            fprintf(stderr, "N must be > 0\n");
        MPI_Finalize();
        return 1;
    }

    srand(1234 + rank);

    float *x = make_random(N);

    double local_sum = 0.0;
    for (int i = 0; i < N; i++)
        local_sum += x[i];

    double global_sum = 0.0;
    MPI_Allreduce(&local_sum, &global_sum, 1,
                  MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    long long total_elements = (long long)N * nproc;
    double mean = global_sum / (double)total_elements;

    double local_sq_diff = 0.0;
    for (int i = 0; i < N; i++) {
        double diff = x[i] - mean;
        local_sq_diff += diff * diff;
    }

    double global_sq_diff = 0.0;
    MPI_Reduce(&local_sq_diff, &global_sq_diff, 1,
               MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double variance = global_sq_diff / (double)total_elements;
        double stddev = sqrt(variance);
        printf("Mean = %.6f\n", mean);
        printf("Standard Deviation = %.6f\n", stddev);
    }

    free(x);
    MPI_Finalize();
    return 0;
}
