#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Get the index of the first occurrence of a character.
 *
 * @param str The string to be searched
 * @param chr The character to find
 * @return char* Pointer to the character in the string, or NULL if there is none
 */
char *strchr(const char *str, int chr) {
    const char *where = str;

    while (1) {
        if (*where == (char) chr) {
            return (char *) where;
        } else if (*where == '\0') {
            return NULL;
        }

        where++;
    }
}

/**
 * @brief Compare two strings with a limit to how many characters to compare.
 *
 * @param lhs First string to be compared
 * @param rhs Second string to be compared
 * @param count Number of characters to compare
 * @return int 0 if both strings are equal up until the limit, <0 if lhs is lower than rhs, >0 if
 * rhs is lower than lhs
 */
int strncmp(const char *lhs, const char *rhs, size_t count) {
    while (count--) {
        if (*lhs++ != *rhs++) {
            return *(uint8_t *) (lhs - 1) - *(uint8_t *) (rhs - 1);
        }
    }
    return 0;
}

/**
 * @brief Calculate the length of a string.
 *
 * @param str A zero terminated string
 * @return size_t The number of characters until \0
 */
size_t strlen(const char *str) {
    size_t i = 0;
    while (*str++ != '\0')
        i++;
    return i;
}

/**
 * @brief Transform a character into its lowercase form if it has one.
 *
 * @param c The character to be transformed
 * @return int The lowercase form of the character or the same character if it has none
 */
int tolower(int c) {
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    return c;
}

/**
 * @brief Transform a character into its uppercase form if it has one.
 *
 * @param c The character to be transformed
 * @return int The uppercase form of the character or the same character if it has none
 */
int toupper(int c) {
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 'A';
    return c;
}

int isspace(int c) {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v';
}

int isalpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isnum(int c) {
    return c >= '0' && c <= '9';
}

int isalphanum(int c) {
    return isalpha(c) || isnum(c);
}

/**
 * @brief Set all bytes of a memory region to a single value.
 *
 * @param dest Destination memory address
 * @param ch Value to be written to destination bytes. Is cast to uint8_t
 * @param count Number of bytes to write
 * @return void* Copy of dest
 */
void *memset(void *dest, int ch, size_t count) {
    uint8_t  b       = ch;
    uint8_t *dest_u8 = dest;
    while (count--) {
        *dest_u8++ = b;
    }
    return dest;
}

/**
 * @brief Copy count bytes from src to dest.
 *
 * @param dest Destination memory address
 * @param src Source memory address
 * @param count Number of bytes to copy
 * @return void* Copy of dest
 */
void *memcpy(void *dest, const void *src, size_t count) {
    const uint8_t *src_u8  = src;
    uint8_t *      dest_u8 = dest;

    while (count--) {
        *dest_u8++ = *src_u8++;
    }

    return dest;
}

/**
 * @brief Create an ASCII representation of a number.
 *
 * @param num Number to be represented
 * @param str String buffer to write the representation into
 * @param base Base to write the number on
 * @return char* Copy of str
 */
char *itoa(int num, char *str, int base) {
    uint8_t neg   = 0;
    char *  start = str;

    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }
    str[0] = '\0';
    str++;

    if (num < 0 && base == 10) {
        neg = 1;
        num = -num;
    }

    while (num > 0) {
        *str++ = "0123456789abcdef"[num % base];
        num    = num / base;
    }

    if (neg) {
        *str++ = '-';
    }

    char *orig = start;
    char  s;
    while (start < str) {
        s        = *(--str);
        *str     = *start;
        *start++ = s;
    }

    return orig;
}
