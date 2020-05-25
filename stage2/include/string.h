#pragma once

#include <stddef.h>

char * strchr(const char *str, int chr);
int    strncmp(const char *lhs, const char *rhs, size_t count);
size_t strlen(const char *str);
void * memcpy(void *dest, const void *src, size_t count);
int    tolower(int c);
int    toupper(int c);
char * itoa(int num, char *str, int base);
