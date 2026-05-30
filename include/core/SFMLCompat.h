#ifndef CORE_SFMLCOMPAT_H
#define CORE_SFMLCOMPAT_H

// Workaround for SFML 2.6.2 compatibility with clang/libc++ in C++17/20 mode.
// SFML uses std::basic_string<Uint32>::iterator which requires
// std::char_traits<unsigned int>. This specialization is non-standard
// and is not provided by libc++ in C++17/20 mode.

#include <cstddef>
#include <string>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

// Provide the missing char_traits<unsigned int> specialization
namespace std {

template <>
struct char_traits<unsigned int> {
    using char_type  = unsigned int;
    using int_type   = unsigned int;
    using off_type   = streamoff;
    using pos_type   = streampos;
    using state_type = mbstate_t;

    static void assign(char_type& c1, const char_type& c2) noexcept { c1 = c2; }
    static bool eq(char_type a, char_type b) noexcept { return a == b; }
    static bool lt(char_type a, char_type b) noexcept { return a < b; }

    static int compare(const char_type* s1, const char_type* s2, size_t n) {
        for (size_t i = 0; i < n; ++i)
            if (!eq(s1[i], s2[i])) return lt(s1[i], s2[i]) ? -1 : 1;
        return 0;
    }

    static size_t length(const char_type* s) {
        size_t len = 0;
        while (*s++) ++len;
        return len;
    }

    static const char_type* find(const char_type* s, size_t n, const char_type& a) {
        for (size_t i = 0; i < n; ++i)
            if (eq(s[i], a)) return s + i;
        return nullptr;
    }

    static char_type* move(char_type* s1, const char_type* s2, size_t n) {
        if (n == 0) return s1;
        return static_cast<char_type*>(memmove(s1, s2, n * sizeof(char_type)));
    }

    static char_type* copy(char_type* s1, const char_type* s2, size_t n) {
        if (n == 0) return s1;
        return static_cast<char_type*>(memcpy(s1, s2, n * sizeof(char_type)));
    }

    static char_type* assign(char_type* s, size_t n, char_type a) {
        for (size_t i = 0; i < n; ++i) s[i] = a;
        return s;
    }

    static int_type not_eof(int_type c) noexcept {
        return eq_int_type(c, eof()) ? ~eof() : c;
    }

    static char_type to_char_type(int_type c) noexcept { return static_cast<char_type>(c); }
    static int_type to_int_type(char_type c) noexcept { return static_cast<int_type>(c); }
    static bool eq_int_type(int_type a, int_type b) noexcept { return a == b; }
    static int_type eof() noexcept { return static_cast<int_type>(-1); }
};

} // namespace std

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // CORE_SFMLCOMPAT_H
