#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define EOF ((int)-1)

typedef struct data_buffer {
    size_t size;
    size_t current_pos;
    void *data;
} data_buffer_t;

typedef struct entry {
    union {
        struct {
            char *name;
            uint32_t size;
            bool isdir;
            bool populated;
            uint32_t cluster;
            struct entry *children;
        };
        char null;
    };
} entry_t;

// Files are read only and have limited functionality
typedef struct file {
    uint8_t id;
    entry_t *entry;

    uint32_t pos;
} file_t;
