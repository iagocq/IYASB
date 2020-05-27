#include <config.h>
#include <disk.h>
#include <fat32.h>
#include <printf.h>
#include <screen.h>
#include <string.h>

void file_tree(entry_t *entry, int depth);
void populate_files(entry_t *dir);

/**
 * @brief Entry point for the C code.
 */
void centry(uint8_t drive_number) {
    init_screen();
    init_disk(drive_number);
    init_fat();

    char *filename = "/boot/iyasb.cfg";

    file_t *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error: failed to open %s\n", filename);
        return;
    }

    printf("%s is %d bytes long\n", filename, fsize(file));

    config_t config;

    size_t read = fread(&config, sizeof(config_t), 1, file);
    memdump(&config, sizeof(config_t), 20);
    if (read != 1) {
        printf("Error: failed to read config\n");
        return;
    }

    if (fclose(file) == EOF) {
        printf("Error: failed to close the file\n");
    }

    extern entry_t root;
    file_tree(&root, 0);
}

void file_tree(entry_t *entry, int depth) {
    for (int i = 0; i < depth - 1; i++) {
        printf("  ");
    }

    if (depth > 0) {
        if (entry->next_sibling == NULL) {
            // last child
            printf("%c", 0xc0);
        } else {
            printf("%c", 0xc3);
        }
        printf(" ");
    }

    printf("%s", entry->name);
    if (!entry->populated) {
        populate_files(entry);
    }

    for (entry_t *child = entry->first_child; child != NULL; child = child->next_sibling) {
        if (strncmp(child->name, ".", 1) == 0 || strncmp(child->name, "..", 2) == 0) {
            continue;
        }
        printf("\n");
        file_tree(child, depth + 1);
    }

    if (depth == 0) {
        printf("\n");
    }
}
