#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

// Global variables
int global_var = 42;
const char* global_string = "Hello, LLVM!";

// Structure definition
typedef struct {
    int x;
    float y;
    char z;
    void* ptr;
} MyStruct;

// Function prototypes
int factorial(int n);
float compute_average(float* arr, int size);
void modify_struct(MyStruct* s);
int binary_search(int* arr, int size, int target);
unsigned int process_flags(unsigned int flags, int option);
void print_values(int count, ...);
int safe_add(int a, int b);

// Safe addition function (replaces inline assembly for portability)
int safe_add(int a, int b) {
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b)) {
        fprintf(stderr, "Integer overflow detected in addition\n");
        return 0;
    }
    return a + b;
}

// Recursive function with input validation
int factorial(int n) {
    if (n < 0) {
        fprintf(stderr, "Factorial of negative number is undefined\n");
        return 0;
    }
    if (n <= 1) return 1;
    
    // Check for potential overflow
    if (n > 12) { // 13! overflows 32-bit int
        fprintf(stderr, "Factorial of %d would cause integer overflow\n", n);
        return 0;
    }
    
    return n * factorial(n - 1);
}

// Array manipulation with pointer arithmetic
float compute_average(float* arr, int size) {
    if (arr == NULL || size <= 0) {
        fprintf(stderr, "Invalid array or size\n");
        return 0.0f;
    }
    
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += *(arr + i);  // Pointer arithmetic
    }
    return sum / size;
}

// Structure manipulation with proper memory management
void modify_struct(MyStruct* s) {
    if (s == NULL) {
        fprintf(stderr, "Null struct pointer\n");
        return;
    }
    
    s->x += 10;
    s->y *= 2.5f;
    s->z = 'X';
    
    // Free existing memory if any
    if (s->ptr != NULL) {
        free(s->ptr);
    }
    
    // Allocate new memory
    s->ptr = malloc(sizeof(int) * 5);
    if (s->ptr != NULL) {
        memset(s->ptr, 0, sizeof(int) * 5);  // Initialize to zero
    } else {
        fprintf(stderr, "Memory allocation failed\n");
    }
}

// Binary search algorithm (demonstrates branching and loops)
int binary_search(int* arr, int size, int target) {
    if (arr == NULL || size <= 0) {
        fprintf(stderr, "Invalid array or size\n");
        return -1;
    }
    
    int left = 0;
    int right = size - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (arr[mid] == target)
            return mid;
        
        if (arr[mid] < target)
            left = mid + 1;
        else
            right = mid - 1;
    }
    
    return -1;  // Not found
}

// Function with switch statement and bitwise operations
unsigned int process_flags(unsigned int flags, int option) {
    unsigned int result = flags;
    
    switch (option) {
        case 0:
            result |= (1U << 0);  // Set bit 0
            break;
        case 1:
            result &= ~(1U << 1); // Clear bit 1
            break;
        case 2:
            result ^= (1U << 2);  // Toggle bit 2
            break;
        case 3:
            result = ~result;     // Invert all bits
            break;
        default:
            result = (result << 1) | (result >> (sizeof(unsigned int)*CHAR_BIT - 1)); // Rotate left by 1
    }
    
    return result;
}

// Variable arguments function
void print_values(int count, ...) {
    va_list args;
    va_start(args, count);
    
    printf("Printing %d values:\n", count);
    for (int i = 0; i < count; i++) {
        int value = va_arg(args, int);
        printf("Value %d: %d\n", i, value);
    }
    
    va_end(args);
}

int main() {
    // Local variables of different types
    int local_int = 123;
    float local_float = 3.14159f;
    double local_double = 2.71828;
    char local_char = 'A';
    int* local_ptr = &local_int;
    
    // Array declaration and initialization
    int numbers[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    float floats[5] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};
    
    // Conditional statement
    if (local_int > 100) {
        printf("local_int is greater than 100\n");
    } else {
        printf("local_int is not greater than 100\n");
    }
    
    // Loop examples
    printf("For loop output:\n");
    for (int i = 0; i < 5; i++) {
        printf("%d ", i);
    }
    printf("\n");
    
    printf("While loop output:\n");
    int j = 0;
    while (j < 5) {
        printf("%d ", j);
        j++;
    }
    printf("\n");
    
    printf("Do-while loop output:\n");
    int k = 0;
    do {
        printf("%d ", k);
        k++;
    } while (k < 5);
    printf("\n");
    
    // Function calls
    printf("Factorial of 5: %d\n", factorial(5));
    printf("Average of floats: %.2f\n", compute_average(floats, 5));
    
    // Structure usage
    MyStruct my_struct = {10, 20.5f, 'Z', NULL};
    modify_struct(&my_struct);
    printf("Modified struct: x=%d, y=%.2f, z=%c\n", 
           my_struct.x, my_struct.y, my_struct.z);
    
    // Binary search
    int sorted_array[10] = {11, 22, 33, 44, 55, 66, 77, 88, 99, 100};
    int index = binary_search(sorted_array, 10, 55);
    printf("Index of 55: %d\n", index);
    
    // Bitwise operations
    unsigned int flags = 0x12345678;
    unsigned int processed = process_flags(flags, 2);
    printf("Original flags: 0x%08X, Processed: 0x%08X\n", flags, processed);
    
    // Pointer arithmetic and memory operations
    int* dynamic_array = (int*)malloc(sizeof(int) * 5);
    if (dynamic_array != NULL) {
        for (int i = 0; i < 5; i++) {
            dynamic_array[i] = i * 10;
        }
        
        printf("Dynamic array contents: ");
        for (int i = 0; i < 5; i++) {
            printf("%d ", dynamic_array[i]);
        }
        printf("\n");
        
        free(dynamic_array);
    } else {
        fprintf(stderr, "Failed to allocate dynamic array\n");
    }
    
    // Free struct memory
    if (my_struct.ptr != NULL) {
        free(my_struct.ptr);
    }
    
    // Safe addition call
    int add_result = safe_add(10, 20);
    printf("Safe add result: %d\n", add_result);
    
    // Variable arguments example
    print_values(4, 10, 20, 30, 40);
    
    // Test edge cases
    printf("Testing edge cases:\n");
    printf("Factorial of -5: %d\n", factorial(-5));
    printf("Factorial of 13: %d\n", factorial(13));
    printf("Average of NULL array: %.2f\n", compute_average(NULL, 5));
    
    return 0;
}