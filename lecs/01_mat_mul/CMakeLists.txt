cmake_minimum_required(VERSION 3.10)
project(OptimizedMatrixMultiplication)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Find Intel MKL
find_package(MKL CONFIG REQUIRED)

# Set compiler optimization flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang optimization flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -ffast-math -ftree-vectorize")
    
    # Enable AVX, AVX2, and FMA if available
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-mavx" COMPILER_SUPPORTS_AVX)
    if(COMPILER_SUPPORTS_AVX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
    endif()
    
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
    endif()
    
    check_cxx_compiler_flag("-mfma" COMPILER_SUPPORTS_FMA)
    if(COMPILER_SUPPORTS_FMA)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfma")
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    # Intel compiler specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -xHost -ipo -qopt-zmm-usage=high")
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # MSVC specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /arch:AVX2 /fp:fast /Qvec")
endif()

# Add the executable
add_executable(matrix_multiplication matrix_multiplication.cpp)

# Link with OpenMP and MKL
target_link_libraries(matrix_multiplication PRIVATE OpenMP::OpenMP_CXX MKL::MKL)

# Additional compile options for vectorization reports
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(matrix_multiplication PRIVATE -fopt-info-vec)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    target_compile_options(matrix_multiplication PRIVATE -qopt-report=5 -qopt-report-phase=vec)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(matrix_multiplication PRIVATE -Rpass=loop-vectorize)
endif()

# Print compiler flags
message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")

# Documentation
message(STATUS "
Build instructions:
1. Create a build directory: mkdir build && cd build
2. Run CMake: cmake ..
3. Build the project: make
4. Run the executable: ./matrix_multiplication
")