#include <fat32.h>
#include <string.h>

#ifndef MAX_FILES
#define MAX_FILES 16
#endif

file_t files[MAX_FILES] = {0};
entry_t root = {0};

void populate_files(entry_t *dir) {
    if (!dir->isdir) {
        return;
    }

    // Create buffer guaranteed to be located in the first segment
    uint8_t buffer[4096];

    uint32_t sector = sector_from_cluster(dir->cluster);

    for (int i = 0; i < sectors_per_cluster; i++) {
        read_sector(sector + i, buffer, sector);
    }

    entry_t new_entry;
    do {
        entry_t new_entry = parse_entry(buffer);
    } while (new_entry.null != '\0');
}

file_t *fopen(const char *filename, const char *mode) {
    entry_t *current_entry = &root;

    while (*filename == '/') {
        filename++;
    }

    while (1) {
        // Search for next /
        const char *tmp = strchr(filename, '/');
        if (tmp == NULL) {
            break;
        }

        size_t len = (size_t)(tmp - filename);

        for (current_entry = current_entry->children;
             current_entry != NULL && current_entry->null != '\0';
             current_entry++) {
            if (strncmp(current_entry->name, filename, len) == 0) {
                if (!current_entry->populated) {
                    populate_files(current_entry);
                }
                continue;
            }
        }

        return NULL;
    }

    if (current_entry->isdir) {
        return NULL;
    }

    int i;
    for (i = 0; i < MAX_FILES; i++) {
        if (files[i].id == 0) {
            break;
        }
    }

    if (i == MAX_FILES) {
        return NULL;
    }

    files[i].id = i + 1;
    files[i].entry = current_entry;

    return &files[i];
}

size_t fread(void *buffer, size_t size, size_t count, file_t *stream) {}

int fclose(file_t *stream) {
    int id = stream->id - 1;
    if (files[id].id != stream->id) {
        return EOF;
    }

    files[id].id = 0;
    return 0;
}
