#ifndef __USART_HPP
#define __USART_HPP

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void USART2_Init(void);
void USART2_Send_Char(char chr);
void USART2_Send_Str(const char* str);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
//--------------Realization printf--------------

// Защита от переопределения стандартных макросов
#ifndef INT32_MIN_NUM
#define INT32_MIN_NUM 	 (-2147483647-1)
#define INT32_MAX_NUM 	 (2147483647)
#define UINT32_MAX_NUM   (4294967295U)
#endif

// === Минимальные ручные type traits (без <type_traits>) ===
template<typename T>
struct is_cstring { static constexpr bool value = false; };
template<> struct is_cstring<const char*> { static constexpr bool value = true; };
template<> struct is_cstring<char*> { static constexpr bool value = true; };

template<typename T>
struct is_pointer { static constexpr bool value = false; };
template<typename T> struct is_pointer<T*> { static constexpr bool value = true; };

// Определение знаковости через компиляционную проверку
template<typename T>
struct is_signed {
    static constexpr bool value = static_cast<T>(-1) < static_cast<T>(0);
};

// === Вспомогательные функции вывода ===

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

// === Шаблонная обработка аргументов ===

// Базовый случай — нет аргументов
static inline void format_args(const char*& fmt) {
    while (*fmt) USART2_Send_Char(*fmt++);
}

// Универсальная обработка аргумента с ручными проверками типов
template<typename T>
inline void handle_arg(T arg, char spec) {
    // Строки (проверка через ручной trait)
    if constexpr (is_cstring<T>::value) {
        if (spec == 's') {
            print_str(arg);
            return;
        }
    }
    // Указатели
    else if constexpr (is_pointer<T>::value) {
        if (spec == 'p') {
        	USART2_Send_Char('0');
        	USART2_Send_Char('x');
            print_hex(reinterpret_cast<uintptr_t>(arg), false);
            return;
        }
    }
    // Целочисленные типы
    else if constexpr (sizeof(T) <= sizeof(uint32_t)) {
        // Знаковые типы
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
        // Беззнаковые типы
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
        // Символ (для любого целого типа при спецификаторе 'c')
        if (spec == 'c') {
        	USART2_Send_Char(static_cast<char>(arg));
            return;
        }
    }

    // Несоответствие типа и спецификатора — выводим как есть
    USART2_Send_Char('%');
    USART2_Send_Char(spec);
}

// Рекурсивная обработка аргументов
template<typename T, typename... Args>
inline void format_args(const char*& fmt, T arg, Args... args) {
    while (*fmt) {
        if (*fmt == '%') {
            fmt++; // пропускаем '%'
            char spec = *fmt;

            if (spec == '%') {
            	USART2_Send_Char('%');
            } else {
                handle_arg(arg, spec);
            }

            fmt++;
            // Рекурсивный вызов для следующих аргументов
            format_args(fmt, args...);
            return;
        } else {
        	USART2_Send_Char(*fmt++);
        }
    }

    // Если остались невостребованные аргументы, можно тут реализовать вывод предупреждения
}

// === Публичный интерфейс ===

template<typename... Args>
inline void usart_printf(const char* fmt, Args... args) {
    format_args(fmt, args...);
}

#endif // __cplusplus

#endif /* __USART_HPP */
