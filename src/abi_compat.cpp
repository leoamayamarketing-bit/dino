// ABI compatibility stub for SFML static libraries compiled with older GCC.
// SFML's Err.cpp (in libsfml-system-s.a) was compiled with an older GCC that
// references basic_streambuf::seekpos(std::fpos<int>, ...) which was removed
// in newer versions of libstdc++. We provide a no-op stub so linking works.
// This function is never actually called at runtime.
__asm__(".section .text\n"
        ".globl _ZNSt15basic_streambufIcSt11char_traitsIcEE7seekposESt4fposIiESt13_Ios_Openmode\n"
        "_ZNSt15basic_streambufIcSt11char_traitsIcEE7seekposESt4fposIiESt13_Ios_Openmode:\n"
        "  ret\n");
