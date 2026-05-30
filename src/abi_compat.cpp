// ABI compatibility stubs for SFML static libraries compiled with older GCC.

// SFML's Err.cpp (in libsfml-system-s.a) was compiled with an older GCC that
// references basic_streambuf::seekpos(std::fpos<int>, ...) which was removed
// in newer versions of libstdc++. We provide a no-op stub so linking works.
// This function is never actually called at runtime.
__asm__(".section .text\n"
        ".globl _ZNSt15basic_streambufIcSt11char_traitsIcEE7seekposESt4fposIiESt13_Ios_Openmode\n"
        "_ZNSt15basic_streambufIcSt11char_traitsIcEE7seekposESt4fposIiESt13_Ios_Openmode:\n"
        "  ret\n");

// Old MinGW libraries (libFLAC.a, libvorbis.a) reference the __iob_func()
// symbol for stdin/stdout/stderr, which was removed in modern MinGW-w64
// (WinLibs GCC 16+) in favor of __acrt_iob_func(). This stub bridges the gap.
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <cstdio>
extern "C" {
    FILE* __cdecl __iob_func(void) {
        // Copy the modern CRT's stdin/stdout/stderr into a static array
        // that old libraries expect. The array is returned directly.
        static FILE iob[3];
        static bool initialized = false;
        if (!initialized) {
            iob[0] = *stdin;
            iob[1] = *stdout;
            iob[2] = *stderr;
            initialized = true;
        }
        return iob;
    }
}
#endif
