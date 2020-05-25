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

    char *filename = "/stage2";

    file_t *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Failed to open %s", filename);
    }

    printf("%s is %d bytes long\n", filename, fsize(file));

    if (fclose(file) == EOF) {
        printf("Failed to close the file");
    }

    int char_cols = 13;

    for (int i = 0; i < 0xff; i++) {
        int c = i;
        switch (c) {
            case '\r':
            case '\n':
                c = '!';
                break;
        }
        printf("%02x: %c ", i, c);
        if (i % char_cols == char_cols - 1) {
            printf("\n");
        }
    }

    printf("\n");
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
}
