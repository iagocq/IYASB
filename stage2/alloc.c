#include "alloc.h"
#include <stddef.h>
#include <stdint.h>

extern char _alloc_start_;
extern char _alloc_end_;

void *current_alloc = &_alloc_start_;

inline size_t align_8(size_t bytes);

size_t align_8(size_t bytes) {
    if (bytes % 8 != 0) {
        bytes += 8 - (bytes % 8);
    }

    return bytes;
}

/**
 * @brief Allocate a memory region.
 *
 * @param bytes Number of bytes to allocate
 * @return void* Pointer to the new memory location or NULL if failed to allocate
 */
void *alloc(size_t bytes) {
    bytes = align_8(bytes);

    ptrdiff_t remaining = &_alloc_end_ - (char *) current_alloc;

    if (remaining < bytes) {
        return NULL;
    }

    void *v = current_alloc;
    current_alloc += bytes;

    return v;
}
