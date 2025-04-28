#include <cpuid.h>

#include <bitset>
#include <iostream>
#include <string>
#include <vector>

void check_cpu_features() {
    unsigned int eax, ebx, ecx, edx;

    // Check SSE (SSE4.2 is in ECX bit 20)
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    bool sse42 = (ecx & (1 << 20)) != 0;

    // Check AVX (ECX bit 28)
    bool avx = (ecx & (1 << 28)) != 0;

    // Check AVX2 (EBX bit 5 of CPUID level 7)
    __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx);
    bool avx2 = (ebx & (1 << 5)) != 0;

    // Check AVX-512 (EBX bit 16)
    bool avx512 = (ebx & (1 << 16)) != 0;

    std::cout << "SSE4.2: " << (sse42 ? "Yes" : "No") << "\n";
    std::cout << "AVX:    " << (avx ? "Yes" : "No") << "\n";
    std::cout << "AVX2:   " << (avx2 ? "Yes" : "No") << "\n";
    std::cout << "AVX-512:" << (avx512 ? "Yes" : "No") << "\n";
}

int main() {
    check_cpu_features();
    return 0;
}