# Bentley's Performance Optimization Rules

Jon Bentley, a computer scientist known for his work on algorithm design and programming techniques, developed a set of rules for optimizing code performance. These rules, often called "Bentley's Rules," provide practical approaches to improve program efficiency. This document explains each rule with examples.

## Data Structures

### Packing and Encoding

**Description**: Reduce memory usage by packing data into smaller spaces or using more efficient data encodings.

**Example**:
```c
// Before optimization: Using full integers for boolean flags
int isActive = 1;
int isVisible = 0;
int isSelected = 1;
// 12 bytes for 3 booleans (assuming 4-byte integers)

// After optimization: Using bit fields
unsigned char flags = 0;
#define ACTIVE_FLAG    0x01
#define VISIBLE_FLAG   0x02
#define SELECTED_FLAG  0x04

// Set flags
flags |= ACTIVE_FLAG;   // Set active flag
flags |= SELECTED_FLAG; // Set selected flag

// Check if flag is set
if (flags & ACTIVE_FLAG) {
    // Do something when active
}
// Only 1 byte for all 3 flags
```

### Augmentation

**Description**: Add additional information to data structures to make operations faster.

**Example**:
```java
// Before augmentation: Calculating size every time
class TreeNode {
    TreeNode left;
    TreeNode right;
    int value;
    
    int size() {
        int count = 1; // Count this node
        if (left != null) count += left.size();
        if (right != null) count += right.size();
        return count;
    } // O(n) operation
}

// After augmentation: Storing size in each node
class AugmentedTreeNode {
    AugmentedTreeNode left;
    AugmentedTreeNode right;
    int value;
    int size; // Added field
    
    void updateSize() {
        size = 1;
        if (left != null) {
            size += left.size;
        }
        if (right != null) {
            size += right.size;
        }
    }
    
    int size() {
        return size; // O(1) operation
    }
}
```

### Precomputation

**Description**: Calculate values ahead of time to avoid repeated computation.

**Example**:
```python
# Before precomputation: Calculate square roots on demand
def calculate_distances(points):
    distances = []
    for i in range(len(points)):
        for j in range(i+1, len(points)):
            x_diff = points[j][0] - points[i][0]
            y_diff = points[j][1] - points[i][1]
            distance = math.sqrt(x_diff**2 + y_diff**2)
            distances.append(distance)
    return distances

# After precomputation: Precompute square roots in a lookup table
def initialize_sqrt_table(max_value, precision=1000):
    sqrt_table = [0] * (max_value * precision + 1)
    for i in range(len(sqrt_table)):
        sqrt_table[i] = math.sqrt(i / precision)
    return sqrt_table

# Use precomputed values (assumes values are within table range)
sqrt_table = initialize_sqrt_table(10000)
def get_sqrt(value, precision=1000):
    index = int(value * precision)
    return sqrt_table[index]
```

### Compile-time Initialization

**Description**: Initialize data at compile time rather than runtime.

**Example**:
```c
// Before optimization: Runtime initialization
int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}
// Function called repeatedly during program execution

// After optimization: Compile-time initialization
#define MAX_FACTORIAL 20
const int FACTORIALS[MAX_FACTORIAL + 1] = {
    1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800,
    39916800, 479001600, 6227020800, 87178291200, 1307674368000,
    20922789888000, 355687428096000, 6402373705728000,
    121645100408832000, 2432902008176640000
};

int factorial(int n) {
    if (n <= MAX_FACTORIAL) {
        return FACTORIALS[n];
    } else {
        // Fallback for larger values
        // (or return error if not supported)
    }
}
```

### Caching

**Description**: Store results of expensive computations for later reuse.

**Example**:
```python
# Before caching: Recalculating Fibonacci numbers
def fibonacci(n):
    if n <= 1:
        return n
    return fibonacci(n-1) + fibonacci(n-2)
# Extremely inefficient for large n

# After caching: Using memoization
def fibonacci_with_cache():
    cache = {}
    
    def fib(n):
        if n in cache:
            return cache[n]
        
        if n <= 1:
            result = n
        else:
            result = fib(n-1) + fib(n-2)
            
        cache[n] = result
        return result
    
    return fib

fib = fibonacci_with_cache()
```

### Lazy Evaluation

**Description**: Delay computation until results are needed.

**Example**:
```python
# Before lazy evaluation: Calculate all values upfront
def get_all_data():
    expensive_data1 = compute_expensive_data1()
    expensive_data2 = compute_expensive_data2()
    expensive_data3 = compute_expensive_data3()
    return {
        'data1': expensive_data1,
        'data2': expensive_data2,
        'data3': expensive_data3
    }

# After lazy evaluation: Calculate values only when needed
class LazyData:
    def __init__(self):
        self._data1 = None
        self._data2 = None
        self._data3 = None
    
    @property
    def data1(self):
        if self._data1 is None:
            self._data1 = compute_expensive_data1()
        return self._data1
    
    @property
    def data2(self):
        if self._data2 is None:
            self._data2 = compute_expensive_data2()
        return self._data2
    
    @property
    def data3(self):
        if self._data3 is None:
            self._data3 = compute_expensive_data3()
        return self._data3
```

### Sparsity

**Description**: Use specialized data structures for sparse data to save memory and computation.

**Example**:
```python
# Before optimization: Dense matrix representation
# Full 1000x1000 matrix, but only has a few non-zero values
dense_matrix = [[0 for _ in range(1000)] for _ in range(1000)]
dense_matrix[2][5] = 7
dense_matrix[100][200] = 3
dense_matrix[500][600] = 14

# After optimization: Sparse matrix representation
sparse_matrix = {}  # Dictionary with (row, col) as key
sparse_matrix[(2, 5)] = 7
sparse_matrix[(100, 200)] = 3
sparse_matrix[(500, 600)] = 14

# Accessing elements
def get_value(row, col):
    return sparse_matrix.get((row, col), 0)  # Default to 0 if key doesn't exist
```

## Loops

### Hoisting

**Description**: Move computations outside a loop when they don't change within the loop.

**Example**:
```java
// Before hoisting: Calculation inside loop
for (int i = 0; i < items.length; i++) {
    double scaledValue = items[i] * Math.sqrt(scaleFactor) * (2 * Math.PI);
    processValue(scaledValue);
}

// After hoisting: Move constant calculations outside the loop
double constFactor = Math.sqrt(scaleFactor) * (2 * Math.PI);
for (int i = 0; i < items.length; i++) {
    double scaledValue = items[i] * constFactor;
    processValue(scaledValue);
}
```

### Sentinels

**Description**: Add special values at the end of data structures to eliminate boundary checks within loops.

**Example**:
```c
// Before sentinel: Boundary check on every iteration
int findElement(int* array, int size, int target) {
    for (int i = 0; i < size; i++) {
        if (array[i] == target) {
            return i;
        }
    }
    return -1;
}

// After sentinel: Eliminate boundary check
int findElementWithSentinel(int* array, int size, int target) {
    int last = array[size - 1];  // Save the last element
    array[size - 1] = target;    // Place sentinel
    
    int i = 0;
    while (array[i] != target) {
        i++;
    }
    
    array[size - 1] = last;      // Restore original value
    
    if (i < size - 1 || array[size - 1] == target) {
        return i;
    }
    return -1;
}
```

### Loop Unrolling

**Description**: Execute multiple iterations of a loop simultaneously to reduce loop overhead.

**Example**:
```c
// Before unrolling: Standard loop
sum = 0;
for (i = 0; i < 1000; i++) {
    sum += data[i];
}

// After unrolling: Process 4 elements per iteration
sum = 0;
for (i = 0; i < 1000; i += 4) {
    sum += data[i];
    sum += data[i+1];
    sum += data[i+2];
    sum += data[i+3];
}

// Handle remaining elements if size not divisible by 4
for (; i < 1000; i++) {
    sum += data[i];
}
```

### Loop Fusion

**Description**: Combine multiple loops that iterate over the same range into a single loop.

**Example**:
```java
// Before fusion: Separate loops
// First loop - calculate sum
for (int i = 0; i < data.length; i++) {
    sum += data[i];
}

// Second loop - calculate squared sum
for (int i = 0; i < data.length; i++) {
    sumSquared += data[i] * data[i];
}

// After fusion: Combined loop
for (int i = 0; i < data.length; i++) {
    sum += data[i];
    sumSquared += data[i] * data[i];
}
```

### Eliminating Wasted Iterations

**Description**: Restructure loops to avoid unnecessary iterations or exit early when possible.

**Example**:
```java
// Before optimization: Process all elements
boolean containsNegative(int[] array) {
    boolean found = false;
    for (int i = 0; i < array.length; i++) {
        if (array[i] < 0) {
            found = true;
        }
    }
    return found;
}

// After optimization: Exit as soon as condition is met
boolean containsNegative(int[] array) {
    for (int i = 0; i < array.length; i++) {
        if (array[i] < 0) {
            return true;
        }
    }
    return false;
}
```

## Logic

### Constant Folding and Propagation

**Description**: Evaluate expressions with constant values at compile time rather than runtime.

**Example**:
```c
// Before optimization
double calculateArea(double radius) {
    return 3.14159265358979 * radius * radius;
}

// After constant folding
#define PI 3.14159265358979
double calculateArea(double radius) {
    return PI * radius * radius;
}

// After constant propagation by compiler
// The compiler might optimize this to:
double calculateArea(double radius) {
    return 3.14159265358979 * radius * radius;
    // Or even further to:
    // temp = radius * radius;
    // return 3.14159265358979 * temp;
}
```

### Common-subexpression Elimination

**Description**: Compute a repeated subexpression once and reuse the result.

**Example**:
```java
// Before elimination: Repeated calculations
double distance = Math.sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
double direction = Math.atan2((y2-y1), (x2-x1));

// After elimination: Calculate common parts once
double dx = x2 - x1;
double dy = y2 - y1;
double distance = Math.sqrt(dx*dx + dy*dy);
double direction = Math.atan2(dy, dx);
```

### Algebraic Identities

**Description**: Use mathematical rules to simplify expressions or make them more efficient.

**Example**:
```java
// Before optimization: Expensive operation
double result = Math.pow(x, 2);

// After optimization: Using algebraic identity
double result = x * x;

// Another example - using logarithmic identities
// Before
double result = Math.pow(a, n) * Math.pow(a, m);

// After
double result = Math.pow(a, n + m);
```

### Short-circuiting

**Description**: Take advantage of early termination in conditional expressions.

**Example**:
```java
// Before optimization: Both functions always execute
if (isValidInput(data) && processData(data)) {
    // ...
}

// After optimization with short-circuiting:
// processData() only executes if isValidInput() returns true
if (isValidInput(data) && processData(data)) {
    // ...
}

// Ordering matters for performance! If isValidInput is faster
// and more likely to be false, this ordering is more efficient
```

### Ordering Tests

**Description**: Reorder conditions to execute the most likely to succeed (or fail) first.

**Example**:
```java
// Before optimization: No consideration for probability
if (uncommonCondition() && commonCondition()) {
    // Process
}

// After optimization: Check more likely condition first
if (commonCondition() && uncommonCondition()) {
    // Process
}

// Another example with multiple conditions
// Before optimization: Random order
if (conditionA() && conditionB() && conditionC() && conditionD()) {
    // ...
}

// After optimization: Ordered by probability of failure (lowest to highest)
// and computational cost (cheapest first)
if (cheapAndOftenFalse() && moderateCostAndSometimesFalse() && 
    expensiveButRarelyFalse() && veryExpensiveRarelyFalse()) {
    // ...
}
```

### Creating a Fast Path

**Description**: Optimize for the most common case by providing a fast execution path.

**Example**:
```java
// Before optimization: Treating all cases equally
int binarySearch(int[] array, int target) {
    int left = 0;
    int right = array.length - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (array[mid] == target) {
            return mid;
        }
        
        if (array[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;
}

// After optimization: Check common values first
int binarySearchWithFastPath(int[] array, int target) {
    // Fast path for common values
    if (target == array[0]) {
        return 0;
    }
    if (target == array[array.length - 1]) {
        return array.length - 1;
    }
    
    // Regular binary search for other values
    int left = 0;
    int right = array.length - 1;
    // ... same as before
}
```

### Combining Tests

**Description**: Combine multiple related tests into a single test when possible.

**Example**:
```c
// Before combining: Multiple separate conditions
if (x == 0) {
    return "zero";
} else if (x == 1) {
    return "one";
} else if (x == 2) {
    return "two";
} else if (x == 3) {
    return "three";
}

// After combining: Switch statement
switch (x) {
    case 0: return "zero";
    case 1: return "one";
    case 2: return "two";
    case 3: return "three";
    default: return "other";
}

// Another example - range check
// Before
if (x < 0 || x >= 100) {
    // Out of bounds
}

// After - combined test
if (x >= 0 && x < 100) {
    // In bounds
} else {
    // Out of bounds
}
```

## Functions

### Inlining

**Description**: Replace a function call with the function's body to eliminate call overhead.

**Example**:
```c
// Before inlining: Function call
int square(int x) {
    return x * x;
}

int sum_of_squares(int a, int b) {
    return square(a) + square(b);
}

// After inlining: Function body replaces call
int sum_of_squares_inlined(int a, int b) {
    return (a * a) + (b * b);
}
```

### Tail-recursion Elimination

**Description**: Convert tail-recursive functions into iterative form to avoid stack overflow.

**Example**:
```javascript
// Before elimination: Tail-recursive factorial
function factorial(n, acc = 1) {
    if (n <= 1) {
        return acc;
    }
    return factorial(n - 1, n * acc);
}

// After elimination: Iterative factorial
function factorialIterative(n) {
    let acc = 1;
    while (n > 1) {
        acc *= n;
        n -= 1;
    }
    return acc;
}
```

### Coarsening Recursion

**Description**: Modify recursive algorithms to do more work before recursing.

**Example**:
```java
// Before coarsening: Fine-grained recursion
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n-1) + fibonacci(n-2);
}

// After coarsening: Handle multiple base cases directly
int fibonacciCoarsened(int n) {
    if (n <= 1) {
        return n;
    }
    if (n == 2) {
        return 1;
    }
    if (n == 3) {
        return 2;
    }
    if (n == 4) {
        return 3;
    }
    if (n == 5) {
        return 5;
    }
    return fibonacciCoarsened(n-1) + fibonacciCoarsened(n-2);
}

// Even better approach - eliminate recursion entirely with dynamic programming
int fibonacciDP(int n) {
    if (n <= 1) {
        return n;
    }
    
    int[] fib = new int[n+1];
    fib[0] = 0;
    fib[1] = 1;
    
    for (int i = 2; i <= n; i++) {
        fib[i] = fib[i-1] + fib[i-2];
    }
    
    return fib[n];
}
```

This document covers Bentley's rules for code optimization with practical examples. Remember that optimization should be applied thoughtfully, prioritizing code readability and maintainability. Premature optimization can lead to unnecessarily complex code, so these techniques should be applied when there's a genuine performance need based on profiling and measurements.