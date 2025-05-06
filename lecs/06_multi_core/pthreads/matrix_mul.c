#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 1024

// Matrix data structures
double matrixA[MATRIX_SIZE][MATRIX_SIZE];
double matrixB[MATRIX_SIZE][MATRIX_SIZE];
double matrixC_sequential[MATRIX_SIZE][MATRIX_SIZE];
double matrixC_parallel[MATRIX_SIZE][MATRIX_SIZE];

// Thread function arguments
typedef struct {
    int start_row;
    int end_row;
} ThreadArgs;

// Initialize matrix with random values
void initialize_matrices() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrixA[i][j] = (double)rand() / RAND_MAX;
            matrixB[i][j] = (double)rand() / RAND_MAX;
            matrixC_sequential[i][j] = 0.0;
            matrixC_parallel[i][j] = 0.0;
        }
    }
}

// Sequential matrix multiplication
void sequential_matrix_mul() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrixC_sequential[i][j] = 0.0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                matrixC_sequential[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }
}

// Thread function for parallel matrix multiplication
void* parallel_matrix_mul(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int start_row = args->start_row;
    int end_row = args->end_row;

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrixC_parallel[i][j] = 0.0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                matrixC_parallel[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }

    pthread_exit(NULL);
}

// Verify results
int verify_results() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            if (fabs(matrixC_sequential[i][j] - matrixC_parallel[i][j]) >
                0.000001) {
                return 0;  // Not equal
            }
        }
    }
    return 1;  // Equal
}

int main(int argc, char* argv[]) {
    int num_threads = 8;  // Default number of threads

    // Check if number of threads is provided as command line argument
    if (argc > 1) {
        num_threads = atoi(argv[1]);
        if (num_threads <= 0) {
            printf("Invalid number of threads. Using default (4).\n");
            num_threads = 4;
        }
    }

    printf("Matrix Size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Number of threads: %d\n", num_threads);

    srand(time(NULL));
    initialize_matrices();

    // ====== 新的时间测量变量 ======
    struct timespec start_time, end_time;
    double elapsed_time;

    // ====== 串行乘法 ======
    printf("\nPerforming sequential matrix multiplication...\n");
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    sequential_matrix_mul();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                   (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("Sequential execution time: %.6f seconds\n", elapsed_time);

    // ====== 并行乘法 ======
    printf("\nPerforming parallel matrix multiplication with %d threads...\n",
           num_threads);
    pthread_t threads[num_threads];
    ThreadArgs thread_args[num_threads];
    int rows_per_thread = MATRIX_SIZE / num_threads;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = 0; i < num_threads; i++) {
        thread_args[i].start_row = i * rows_per_thread;
        thread_args[i].end_row =
            (i == num_threads - 1) ? MATRIX_SIZE : (i + 1) * rows_per_thread;
        pthread_create(&threads[i], NULL, parallel_matrix_mul,
                       (void*)&thread_args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                   (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("Parallel execution time: %.6f seconds\n", elapsed_time);

    // ====== 结果验证 ======
    printf("\nVerifying results...\n");
    if (verify_results()) {
        printf("Results match! The parallel implementation is correct.\n");
    } else {
        printf(
            "Results do not match! There is an error in the implementation.\n");
    }

    return 0;
}
