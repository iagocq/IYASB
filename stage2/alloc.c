#include <alloc.h>
#include <stddef.h>
#include <stdint.h>

extern char _alloc_start_;
extern char _alloc_end_;

void *current_alloc = &_alloc_start_;

inline size_t align_8(size_t bytes) {
    if (bytes % 8 != 0) {
        bytes += 8 - (bytes % 8);
    }

    return bytes;
}

void *alloc(size_t bytes) {
    bytes = align_8(bytes);

    ptrdiff_t diff = &_alloc_end_ - (char *)current_alloc;

    if (diff < bytes) {
        return NULL;
    }

    void *v = current_alloc;
    current_alloc += bytes;

    return v;
}
