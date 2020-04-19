#include <stddef.h>
#include <stdint.h>
#include <string.h>

char *strchr(const char *str, int chr) {
    const char *where = str;

    while (1) {
        if (*where == (char)chr) {
            return (char *)where;
        } else if (*where == '\0') {
            return NULL;
        }

        where++;
    }
}

int strncmp(const char *lhs, const char *rhs, size_t count) {
    while (count--) {
        if (*lhs++ != *rhs++) {
            return *(uint8_t *)(lhs - 1) - *(uint8_t *)(rhs - 1);
        }
    }
    return 0;
}
