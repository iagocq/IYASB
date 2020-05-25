#include <alloc.h>
#include <disk.h>
#include <fat32.h>
#include <string.h>

#ifndef MAX_FILES
#define MAX_FILES 16
#endif

file_t   files[MAX_FILES] = {0};
entry_t  root             = {0};
int      sectors_per_cluster;
int      entries_per_sector;
uint32_t fat_start;
uint32_t data_start;
uint8_t  fat_cache[4096];
uint32_t current_sector;
uint32_t root_dir_cluster;

void populate_files(entry_t *dir);

/**
 * @brief Initiate FAT operations.
 *
 */
void init_fat() {
    root_dir_cluster = 2;
    read_sector(0, fat_cache);
    fat32_bootsector_t *bs  = (fat32_bootsector_t *) fat_cache;
    fat32_bpb_t *       bpb = &bs->fat32_bpb;

    uint32_t sectors_per_lsector = bpb->bytes_per_lsector / bytes_per_sector;
    sectors_per_cluster          = bpb->lsectors_per_cluster * sectors_per_lsector;
    entries_per_sector           = bytes_per_sector / 0x20;

    fat_start = bpb->reserved_lsectors * sectors_per_lsector;
    data_start =
        (bpb->reserved_lsectors + bpb->lsectors_per_fat * bpb->num_fat) * sectors_per_lsector;
    root_dir_cluster = bpb->root_cluster;

    root.cluster = root_dir_cluster;
    root.name    = "/";
    root.isdir   = true;
    populate_files(&root);
}

/**
 * @brief Get the next cluster in a cluster chain.
 *
 * @param cluster The current cluster number in the chain.
 * @return uint32_t The next cluster number or 0 if on end of chain or invalid
 */
uint32_t next_cluster(uint32_t cluster) {
    uint32_t sector_offset = (cluster * 4) / bytes_per_sector;

    uint32_t real_fat_sector = fat_start + sector_offset;
    if (real_fat_sector != current_sector) {
        current_sector = real_fat_sector;
        read_sector(current_sector, fat_cache);
    }
    uint32_t next_cluster_num = ((uint32_t *) fat_cache)[cluster % (bytes_per_sector / 4)];
    if (next_cluster_num >= 0x0FFFFFF8) {
        return 0;
    }
    return next_cluster_num;
}

/**
 * @brief Calculate the physical sector from a cluster.
 *
 * @param cluster Cluster number
 * @return uint32_t Physical sector number
 */
uint32_t sector_from_cluster(uint32_t cluster) {
    return data_start + (cluster - root_dir_cluster) * sectors_per_cluster;
}

/**
 * @brief Parse the first valid entry given a buffer.
 *
 * @param buffer Pointer to a buffer that will have its value altered to point to the next entry
 * @return entry_t* New entry
 */
entry_t *parse_entry(uint8_t **buffer) {
    fat32_entry_t *real_entry;
    while (true) {
        real_entry = (fat32_entry_t *) *buffer;
        if (real_entry->old.filename[0] != 0xE5 && real_entry->old.attributes != 0x0f) {
            break;
        }
        *buffer = *buffer + 0x20;
    }

    if ((*buffer)[0] == 0) {
        return NULL;
    }

    entry_t *entry      = alloc(sizeof(entry_t));
    entry->populated    = false;
    entry->first_child  = NULL;
    entry->next_sibling = NULL;

    int name_len = 8;
    for (; name_len > 0; name_len--) {
        if (real_entry->old.filename[name_len - 1] != ' ') {
            break;
        }
    }

    int ext_len = 3;
    for (; ext_len > 0; ext_len--) {
        if (real_entry->old.filename[ext_len + 8 - 1] != ' ') {
            break;
        }
    }

    entry->name = alloc(name_len + ext_len + 2);

    if (real_entry->old.filename[0] == 0x05) {
        real_entry->old.filename[0] = 0xE5;
    }

    int str_i = 0;
    for (int i = 0; i < name_len; i++) {
        entry->name[str_i++] = tolower(real_entry->old.filename[i]);
    }

    if (ext_len > 0) {
        entry->name[str_i++] = '.';
        for (int i = 0; i < ext_len; i++) {
            entry->name[str_i++] = tolower(real_entry->old.filename[i + 8]);
        }
    }
    entry->name[str_i] = '\0';
    entry->isdir       = (real_entry->old.attributes & 0x10) == 0x10;
    entry->size        = real_entry->old.size;
    entry->cluster     = (real_entry->old.cluster_hi << 16) | real_entry->old.cluster_lo;

    *buffer = *buffer + 0x20;
    return entry;
}

/**
 * @brief Cache all entries inside a directory.
 *
 * @param dir Directory to populate
 */
void populate_files(entry_t *dir) {
    dir->populated = true;
    if (!dir->isdir) {
        return;
    }

    // Create buffer guaranteed to be located in the first segment
    uint8_t  buffer[4096];
    uint8_t *buf_ptr = buffer;
    entry_t *current_entry;
    uint32_t cluster = dir->cluster;

    while (1) {

        uint32_t sector = sector_from_cluster(cluster);

        for (int i = 0; i < sectors_per_cluster; i++) {
            buf_ptr = buffer;
            read_sector(sector + i, buffer);

            for (int j = 0; j < entries_per_sector; j++) {
                entry_t *new_entry = parse_entry(&buf_ptr);
                if (dir->first_child == NULL) {
                    dir->first_child = new_entry;
                } else {
                    current_entry->next_sibling = new_entry;
                }
                current_entry = new_entry;
                if (current_entry == NULL) {
                    break;
                }
            }
            if (current_entry == NULL) {
                break;
            }
        }

        cluster = next_cluster(cluster);
        if (cluster == 0 && current_entry == NULL) {
            break;
        }
    }
}

/**
 * @brief Open a file and create a file descriptor.
 *
 * @param filename Path to the file to be open
 * @param mode Mode to open the file in (currently ignored)
 * @return file_t* File descriptor or NULL if couldn't open the file
 */
file_t *fopen(const char *filename, const char *mode) {
    entry_t *current_entry = &root;

    while (1) {
        while (*filename == '/') {
            filename++;
        }

        // Search for next /
        const char *tmp = strchr(filename, '/');
        if (tmp == NULL) {
            size_t total_len = strlen(filename);
            if (total_len == 0) {
                break;
            } else {
                tmp = filename + total_len;
            }
        }

        size_t len = (size_t)(tmp - filename);

        int found = 0;

        for (current_entry = current_entry->first_child;
             current_entry != NULL && current_entry->null != '\0';
             current_entry = current_entry->next_sibling) {
            if (strncmp(current_entry->name, filename, len) == 0) {
                if (!current_entry->populated) {
                    populate_files(current_entry);
                }

                if (filename[len] == '\0') {
                    found = 1;
                }
                break;
            }
        }

        if (found) {
            break;
        }

        if (filename[len] != '\0') {
            filename = filename + len;
            continue;
        } else {
            return NULL;
        }
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

    files[i].id    = i + 1;
    files[i].entry = current_entry;

    return &files[i];
}

/**
 * @brief Read content from a file descriptor.
 *
 * @param buffer Memory buffer to store the content
 * @param size Number of bytes for each element
 * @param count Number of elements to read
 * @param stream File descriptor to read from
 * @return size_t Number of bytes read
 */
size_t fread(void *buffer, size_t size, size_t count, file_t *stream) {
    return 0;
}

/**
 * @brief Close an open file descriptor.
 *
 * @param stream The file descriptor to be closed
 * @return int 0 if was able to close the descriptor, EOF otherwise
 */
int fclose(file_t *stream) {
    int id = stream->id - 1;
    if (files[id].id != stream->id) {
        return EOF;
    }

    files[id].id = 0;
    return 0;
}
