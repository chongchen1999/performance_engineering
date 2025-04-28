#ifndef MATRIX_MULTIPLICATION_H
#define MATRIX_MULTIPLICATION_H

#include <immintrin.h>  // For AVX2 intrinsics
#include <omp.h>        // For OpenMP

#include <thread>
#include <vector>

// Matrix structure
struct Matrix {
    int rows;
    int cols;
    std::vector<double> data;

    Matrix(int r, int c) : rows(r), cols(c), data(r * c, 0.0) {}

    double& at(int r, int c) { return data[r * cols + c]; }

    const double& at(int r, int c) const { return data[r * cols + c]; }
};

// Basic matrix multiplication (for comparison)
Matrix naive_matrix_multiply(const Matrix& A, const Matrix& B) {
    if (A.cols != B.rows) {
        throw std::invalid_argument("Incompatible matrix dimensions");
    }

    Matrix C(A.rows, B.cols);

    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < B.cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < A.cols; k++) {
                sum += A.at(i, k) * B.at(k, j);
            }
            C.at(i, j) = sum;
        }
    }

    return C;
}

// Loop interchange optimization
Matrix loop_interchange_matrix_multiply(const Matrix& A, const Matrix& B) {
    if (A.cols != B.rows) {
        throw std::invalid_argument("Incompatible matrix dimensions");
    }

    Matrix C(A.rows, B.cols);

    // i-k-j loop order for better cache locality
    for (int i = 0; i < A.rows; i++) {
        for (int k = 0; k < A.cols; k++) {
            double a_ik = A.at(i, k);
            for (int j = 0; j < B.cols; j++) {
                C.at(i, j) += a_ik * B.at(k, j);
            }
        }
    }

    return C;
}

// OpenMP parallel loop optimization
Matrix parallel_loop_matrix_multiply(const Matrix& A, const Matrix& B) {
    if (A.cols != B.rows) {
        throw std::invalid_argument("Incompatible matrix dimensions");
    }

    Matrix C(A.rows, B.cols);

#pragma omp parallel for
    for (int i = 0; i < A.rows; i++) {
        for (int k = 0; k < A.cols; k++) {
            double a_ik = A.at(i, k);
            for (int j = 0; j < B.cols; j++) {
                C.at(i, j) += a_ik * B.at(k, j);
            }
        }
    }

    return C;
}

// Tiling optimization
Matrix tiled_matrix_multiply(const Matrix& A, const Matrix& B,
                             int tile_size = 32) {
    if (A.cols != B.rows) {
        throw std::invalid_argument("Incompatible matrix dimensions");
    }

    Matrix C(A.rows, B.cols);

// Use tiling to improve cache locality
#pragma omp parallel for
    for (int i0 = 0; i0 < A.rows; i0 += tile_size) {
        for (int j0 = 0; j0 < B.cols; j0 += tile_size) {
            for (int k0 = 0; k0 < A.cols; k0 += tile_size) {
                // Process tile
                for (int i = i0; i < std::min(i0 + tile_size, A.rows); i++) {
                    for (int k = k0; k < std::min(k0 + tile_size, A.cols);
                         k++) {
                        double a_ik = A.at(i, k);
                        for (int j = j0; j < std::min(j0 + tile_size, B.cols);
                             j++) {
                            C.at(i, j) += a_ik * B.at(k, j);
                        }
                    }
                }
            }
        }
    }

    return C;
}

// Recursive divide and conquer
void matrix_mult_recursive(const double* A, const double* B, double* C,
                           int A_rows, int A_cols, [[maybe_unused]] int B_rows,
                           int B_cols, int lda, int ldb, int ldc,
                           int threshold = 128) {
    int m = A_rows;
    int n = B_cols;
    int k = A_cols;

    // Base case: use simple matrix multiplication
    if (m <= threshold || n <= threshold || k <= threshold) {
        for (int i = 0; i < m; i++) {
            for (int kk = 0; kk < k; kk++) {
                double a_ik = A[i * lda + kk];
                for (int j = 0; j < n; j++) {
                    C[i * ldc + j] += a_ik * B[kk * ldb + j];
                }
            }
        }
        return;
    }

    // Divide matrices into quadrants
    int m2 = m / 2;
    int n2 = n / 2;
    int k2 = k / 2;

    // Pointers to submatrices
    const double* A11 = A;
    const double* A12 = A + k2;
    const double* A21 = A + m2 * lda;
    const double* A22 = A + m2 * lda + k2;

    const double* B11 = B;
    const double* B12 = B + n2;
    const double* B21 = B + k2 * ldb;
    const double* B22 = B + k2 * ldb + n2;

    double* C11 = C;
    double* C12 = C + n2;
    double* C21 = C + m2 * ldc;
    double* C22 = C + m2 * ldc + n2;

// Recursive calls - can be parallelized
#pragma omp task
    matrix_mult_recursive(A11, B11, C11, m2, k2, k2, n2, lda, ldb, ldc,
                          threshold);

#pragma omp task
    matrix_mult_recursive(A11, B12, C12, m2, k2, k2, n - n2, lda, ldb, ldc,
                          threshold);

#pragma omp task
    matrix_mult_recursive(A21, B11, C21, m - m2, k2, k2, n2, lda, ldb, ldc,
                          threshold);

#pragma omp task
    matrix_mult_recursive(A21, B12, C22, m - m2, k2, k2, n - n2, lda, ldb, ldc,
                          threshold);

#pragma omp taskwait

// Second set of recursive multiplications
#pragma omp task
    matrix_mult_recursive(A12, B21, C11, m2, k - k2, k - k2, n2, lda, ldb, ldc,
                          threshold);

#pragma omp task
    matrix_mult_recursive(A12, B22, C12, m2, k - k2, k - k2, n - n2, lda, ldb,
                          ldc, threshold);

#pragma omp task
    matrix_mult_recursive(A22, B21, C21, m - m2, k - k2, k - k2, n2, lda, ldb,
                          ldc, threshold);

#pragma omp task
    matrix_mult_recursive(A22, B22, C22, m - m2, k - k2, k - k2, n - n2, lda,
                          ldb, ldc, threshold);

#pragma omp taskwait
}

Matrix divide_conquer_matrix_multiply(const Matrix& A, const Matrix& B) {
    if (A.cols != B.rows) {
        throw std::invalid_argument("Incompatible matrix dimensions");
    }

    Matrix C(A.rows, B.cols);

#pragma omp parallel
    {
#pragma omp single
        {
            matrix_mult_recursive(A.data.data(), B.data.data(), C.data.data(),
                                  A.rows, A.cols, B.rows, B.cols, A.cols,
                                  B.cols, C.cols);
        }
    }

    return C;
}

// AVX2 intrinsics optimization
Matrix avx2_matrix_multiply(const Matrix& A, const Matrix& B) {
    if (A.cols != B.rows) {
        throw std::invalid_argument("Incompatible matrix dimensions");
    }

    Matrix C(A.rows, B.cols);
    const int k = A.cols;

    // We need to ensure proper alignment for best AVX performance
    // Assuming matrices are properly padded for simplicity

#pragma omp parallel for
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < B.cols; j += 4) {
            __m256d sum = _mm256_setzero_pd();  // Initialize sum to zero

            // Process 4 elements at a time using AVX2
            for (int l = 0; l < k; l++) {
                __m256d a_val =
                    _mm256_set1_pd(A.at(i, l));  // Broadcast A value

                // Load 4 consecutive B values
                if (j + 3 < B.cols) {
                    __m256d b_vals =
                        _mm256_set_pd(B.at(l, j + 3), B.at(l, j + 2),
                                      B.at(l, j + 1), B.at(l, j));

                    // Multiply and accumulate
                    sum = _mm256_add_pd(sum, _mm256_mul_pd(a_val, b_vals));
                } else {
                    // Handle edge case when we don't have 4 elements left
                    for (int jj = j; jj < B.cols; jj++) {
                        C.at(i, jj) += A.at(i, l) * B.at(l, jj);
                    }
                    break;
                }
            }

            // Store the result
            if (j + 3 < B.cols) {
                double temp[4];
                _mm256_storeu_pd(temp, sum);
                C.at(i, j) = temp[0];
                C.at(i, j + 1) = temp[1];
                C.at(i, j + 2) = temp[2];
                C.at(i, j + 3) = temp[3];
            }
        }
    }

    return C;
}

// The most optimized version - combining multiple optimizations
Matrix optimized_matrix_multiply(const Matrix& A, const Matrix& B) {
    if (A.cols != B.rows) {
        throw std::invalid_argument("Incompatible matrix dimensions");
    }

    Matrix C(A.rows, B.cols);
    const int tile_size = 32;  // Adjust based on cache size
    const int k = A.cols;

    // Set number of threads for OpenMP
    int num_threads = std::thread::hardware_concurrency();
    omp_set_num_threads(num_threads);

// Combine tiling with AVX2 and OpenMP
#pragma omp parallel for
    for (int i0 = 0; i0 < A.rows; i0 += tile_size) {
        for (int j0 = 0; j0 < B.cols; j0 += tile_size) {
            for (int k0 = 0; k0 < k; k0 += tile_size) {
                // Process each tile
                for (int i = i0; i < std::min(i0 + tile_size, A.rows); i++) {
                    for (int k1 = k0; k1 < std::min(k0 + tile_size, k); k1++) {
                        double a_ik = A.at(i, k1);
                        __m256d a_vec = _mm256_set1_pd(a_ik);

                        // Use AVX2 for vectorized inner loop
                        for (int j = j0;
                             j < std::min(j0 + tile_size, B.cols) - 3; j += 4) {
                            __m256d b_vec =
                                _mm256_set_pd(B.at(k1, j + 3), B.at(k1, j + 2),
                                              B.at(k1, j + 1), B.at(k1, j));

                            __m256d c_vec =
                                _mm256_loadu_pd(&C.data[i * C.cols + j]);
                            c_vec = _mm256_add_pd(c_vec,
                                                  _mm256_mul_pd(a_vec, b_vec));
                            _mm256_storeu_pd(&C.data[i * C.cols + j], c_vec);
                        }

                        // Handle remaining elements that don't fit in vector
                        // width
                        int j_aligned_end =
                            ((std::min(j0 + tile_size, B.cols)) / 4) * 4;
                        for (int j = j_aligned_end;
                             j < std::min(j0 + tile_size, B.cols); j++) {
                            C.at(i, j) += a_ik * B.at(k1, j);
                        }
                    }
                }
            }
        }
    }

    return C;
}

#endif  // MATRIX_MULTIPLICATION_H