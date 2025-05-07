#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// Sequential quicksort implementation
template <typename T>
void quicksort_seq(std::vector<T>& arr, int left, int right) {
    if (left >= right) {
        return;
    }

    // Choose pivot (median of three)
    int mid = left + (right - left) / 2;
    if (arr[mid] < arr[left]) {
        std::swap(arr[left], arr[mid]);
    }
    if (arr[right] < arr[left]) {
        std::swap(arr[left], arr[right]);
    }
    if (arr[mid] < arr[right]) {
        std::swap(arr[mid], arr[right]);
    }
    T pivot = arr[right];

    // Partition
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (arr[j] <= pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    i++;
    std::swap(arr[i], arr[right]);

    // Recursive calls
    quicksort_seq(arr, left, i - 1);
    quicksort_seq(arr, i + 1, right);
}

// Parallel quicksort implementation
template <typename T>
void quicksort_parallel(std::vector<T>& arr, int left, int right,
                        int depth = 0) {
    // Use sequential sort when the array is small or we've reached max
    // recursion depth
    const int SEQUENTIAL_THRESHOLD = 10000;
    const int MAX_DEPTH = 4;  // Limit thread creation depth

    if (right - left <= SEQUENTIAL_THRESHOLD || depth >= MAX_DEPTH) {
        quicksort_seq(arr, left, right);
        return;
    }

    // Choose pivot (median of three)
    int mid = left + (right - left) / 2;
    if (arr[mid] < arr[left]) {
        std::swap(arr[left], arr[mid]);
    }
    if (arr[right] < arr[left]) {
        std::swap(arr[left], arr[right]);
    }
    if (arr[mid] < arr[right]) {
        std::swap(arr[mid], arr[right]);
    }
    T pivot = arr[right];

    // Partition
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (arr[j] <= pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    i++;
    std::swap(arr[i], arr[right]);

    // Launch a new thread for the left part and do the right part in the
    // current thread
    std::future<void> left_future =
        std::async(std::launch::async, [&arr, left, i, depth]() {
            quicksort_parallel(arr, left, i - 1, depth + 1);
        });

    // Sort the right part
    quicksort_parallel(arr, i + 1, right, depth + 1);

    // Wait for the left part to complete
    left_future.wait();
}

// Function to check if a vector is sorted
template <typename T>
bool is_sorted(const std::vector<T>& arr) {
    return std::is_sorted(arr.begin(), arr.end());
}

// Function to generate a random vector
template <typename T>
std::vector<T> generate_random_vector(size_t size, T min_val, T max_val) {
    std::vector<T> vec(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> dist(min_val, max_val);

    for (size_t i = 0; i < size; ++i) {
        vec[i] = dist(gen);
    }

    return vec;
}

// Benchmark function
template <typename T>
void benchmark(size_t size, T min_val, T max_val, int num_runs = 5) {
    std::cout << "Running benchmark with vector size: " << size << std::endl;

    double total_std_sort = 0.0;
    double total_parallel_sort = 0.0;

    for (int run = 0; run < num_runs; ++run) {
        // Generate random vectors
        std::vector<T> vec_std =
            generate_random_vector<T>(size, min_val, max_val);
        std::vector<T> vec_parallel = vec_std;  // Make a copy

        // Benchmark std::sort
        auto start_std = std::chrono::high_resolution_clock::now();
        std::sort(vec_std.begin(), vec_std.end());
        auto end_std = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_std = end_std - start_std;
        total_std_sort += elapsed_std.count();

        // Benchmark parallel quicksort
        auto start_parallel = std::chrono::high_resolution_clock::now();
        quicksort_parallel(vec_parallel, 0, vec_parallel.size() - 1);
        auto end_parallel = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_parallel =
            end_parallel - start_parallel;
        total_parallel_sort += elapsed_parallel.count();

        // Verify sorting
        bool std_sorted = is_sorted(vec_std);
        bool parallel_sorted = is_sorted(vec_parallel);

        std::cout << "Run " << run + 1 << ":" << std::endl;
        std::cout << "  std::sort:        " << elapsed_std.count()
                  << "s (correctly sorted: " << (std_sorted ? "yes" : "no")
                  << ")" << std::endl;
        std::cout << "  parallel quicksort: " << elapsed_parallel.count()
                  << "s (correctly sorted: " << (parallel_sorted ? "yes" : "no")
                  << ")" << std::endl;

        // Add a small delay between runs
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Calculate and display average times
    double avg_std_sort = total_std_sort / num_runs;
    double avg_parallel_sort = total_parallel_sort / num_runs;
    double speedup = avg_std_sort / avg_parallel_sort;

    std::cout << "\nAverage times over " << num_runs << " runs:" << std::endl;
    std::cout << "  std::sort:        " << avg_std_sort << "s" << std::endl;
    std::cout << "  parallel quicksort: " << avg_parallel_sort << "s"
              << std::endl;
    std::cout << "  Speed up: " << speedup << "x" << std::endl;
}

int main() {
    // Number of hardware threads
    unsigned int num_threads = std::thread::hardware_concurrency();
    std::cout << "Number of hardware threads: " << num_threads << std::endl;

    // Run benchmarks for different sizes
    benchmark<int>(100000, 1, 1000000);
    benchmark<int>(1000000, 1, 1000000);
    benchmark<int>(10000000, 1, 1000000);

    return 0;
}