#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define EOF ((int) -1)

typedef struct data_buffer {
    size_t size;
    size_t current_pos;
    void * data;
} data_buffer_t;

typedef struct entry {
    union {
        struct {
            char *        name;
            uint32_t      size;
            bool          isdir;
            bool          populated;
            uint32_t      cluster;
            struct entry *first_child;
            struct entry *next_sibling;
        };
        char null;
    };
} entry_t;

typedef struct old_entry {
    uint8_t  filename[11];
    uint8_t  attributes;
    uint8_t  useless_1[8];
    uint16_t cluster_hi;
    uint8_t  useless_2[4];
    uint16_t cluster_lo;
    uint32_t size;
} __attribute__((packed)) old_entry_t;

typedef struct lfn_entry {
    uint8_t  seq_num;
    uint16_t name_1[5];
    uint8_t  attributes;
    uint8_t  type;
    uint8_t  checksum;
    uint16_t name_2[6];
    uint16_t cluster;
    uint16_t name_3[2];
} __attribute__((packed)) lfn_entry_t;

typedef struct fat32_entry {
    union {
        old_entry_t old;
        lfn_entry_t lfn;
    };
} fat32_entry_t;

// Files are read only and have limited functionality
typedef struct file {
    uint8_t  id;
    entry_t *entry;

    uint32_t pos;
} file_t;

void init_fat();

file_t *fopen(const char *filename, const char *mode);
size_t  fread(void *buffer, size_t size, size_t count, file_t *stream);
