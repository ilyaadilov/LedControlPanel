#ifndef __USART_HPP
#define __USART_HPP

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void USART2_Init(void);
void USART2_DeInit(void);
void USART2_Send_Char(char chr);
void USART2_Send_Str(const char* str);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
//--------------Realization printf--------------

#ifndef INT32_MIN_NUM
#define INT32_MIN_NUM 	 (-2147483647-1)
#define INT32_MAX_NUM 	 (2147483647)
#define UINT32_MAX_NUM   (4294967295U)
#endif

// === type traits  ===
template<typename T>
struct is_cstring { static constexpr bool value = false; };
template<> struct is_cstring<const char*> { static constexpr bool value = true; };
template<> struct is_cstring<char*> { static constexpr bool value = true; };

template<typename T>
struct is_pointer { static constexpr bool value = false; };
template<typename T> struct is_pointer<T*> { static constexpr bool value = true; };

// Identification of signedness
template<typename T>
struct is_signed {
    static constexpr bool value = static_cast<T>(-1) < static_cast<T>(0);
};

// === Support functions ===

static inline void print_str(const char* str) {
    if (!str) str = "(null)";
    while (*str) USART2_Send_Char(*str++);
}

static inline void print_uint(uint32_t num) {
    if (num == 0) {
    	USART2_Send_Char('0');
        return;
    }
    char buf[16];
    int i = 0;
    while (num) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    while (i--) USART2_Send_Char(buf[i]);
}

static inline void print_int(int32_t num) {
    if (num < 0) {
    	USART2_Send_Char('-');
        uint32_t unum = (num == INT32_MIN_NUM)
            ? static_cast<uint32_t>(INT32_MAX_NUM) + 1
            : static_cast<uint32_t>(-num);
        print_uint(unum);
    } else {
        print_uint(static_cast<uint32_t>(num));
    }
}

static inline void print_hex(uint32_t num, bool upper) {
    const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    if (num == 0) {
    	USART2_Send_Char('0');
        return;
    }
    char buf[16];
    int i = 0;
    while (num) {
        buf[i++] = digits[num & 0xF];
        num >>= 4;
    }
    while (i--) USART2_Send_Char(buf[i]);
}

// === Template processing of arguments ===

// The base case is there are no arguments
static inline void format_args(const char*& fmt) {
    while (*fmt) USART2_Send_Char(*fmt++);
}

// Universal argument handling with manual type checks
template<typename T>
inline void handle_arg(T arg, char spec) {
    // Strings
    if constexpr (is_cstring<T>::value) {
        if (spec == 's') {
            print_str(arg);
            return;
        }
    }
    // Pointers
    else if constexpr (is_pointer<T>::value) {
        if (spec == 'p') {
        	USART2_Send_Char('0');
        	USART2_Send_Char('x');
            print_hex(reinterpret_cast<uintptr_t>(arg), false);
            return;
        }
    }
    // Integer
    else if constexpr (sizeof(T) <= sizeof(uint32_t)) {
        // Signed
        if constexpr (is_signed<T>::value) {
            int32_t val = static_cast<int32_t>(arg);
            if (spec == 'd') {
                print_int(val);
                return;
            } else if (spec == 'u') {
                print_uint(static_cast<uint32_t>(val));
                return;
            } else if (spec == 'x' || spec == 'X') {
                print_hex(static_cast<uint32_t>(val), spec == 'X');
                return;
            }
        }
        // Unsigned
        else {
            uint32_t val = static_cast<uint32_t>(arg);
            if (spec == 'u') {
                print_uint(val);
                return;
            } else if (spec == 'd') {
                print_int(static_cast<int32_t>(val));
                return;
            } else if (spec == 'x' || spec == 'X') {
                print_hex(val, spec == 'X');
                return;
            }
        }
        // symbol ('c')
        if (spec == 'c') {
        	USART2_Send_Char(static_cast<char>(arg));
            return;
        }
    }

    // Type and specifier mismatch
    USART2_Send_Char('%');
    USART2_Send_Char(spec);
}

// Recursive argument processing
template<typename T, typename... Args>
inline void format_args(const char*& fmt, T arg, Args... args) {
    while (*fmt) {
        if (*fmt == '%') {
            fmt++; // miss '%'
            char spec = *fmt;

            if (spec == '%') {
            	USART2_Send_Char('%');
            } else {
                handle_arg(arg, spec);
            }

            fmt++;
            // Recursive call for the following arguments
            format_args(fmt, args...);
            return;
        } else {
        	USART2_Send_Char(*fmt++);
        }
    }

    // If there are unclaimed arguments, you can implement the warning output here.
}

// === Public interface ===

template<typename... Args>
inline void usart_printf(const char* fmt, Args... args) {
    format_args(fmt, args...);
}

#endif // __cplusplus

#endif /* __USART_HPP */
