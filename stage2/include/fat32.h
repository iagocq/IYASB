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

/** Abstract entry over a real entry, with useful information cached for later physical access */
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

/** "Old" FAT32 entry */
typedef struct old_entry {
    uint8_t  filename[11];
    uint8_t  attributes;
    uint8_t  useless_1[8];
    uint16_t cluster_hi;
    uint8_t  useless_2[4];
    uint16_t cluster_lo;
    uint32_t size;
} __attribute__((packed)) old_entry_t;

/** Long filename FAT32 entry */
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

/** Abstraction over the different kinds of physical entries in a FAT directory */
typedef struct fat32_entry {
    union {
        /** "Old" FAT32 view of the entry */
        old_entry_t old;

        /** Long filename view of the entry.
         *  Currently unused */
        lfn_entry_t lfn;
    };
} fat32_entry_t;

// Files are read only and have limited functionality
/** File descriptor structure */
typedef struct file {
    /** Internal descriptor ID */
    uint8_t id;

    /** Pointer to the file entry */
    entry_t *entry;

    /** Current read position */
    uint32_t pos;
} file_t;

void init_fat();

file_t *fopen(const char *filename, const char *mode);
size_t  fread(void *buffer, size_t size, size_t count, file_t *stream);
size_t  fsize(file_t *stream);
