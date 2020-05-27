#include "config.h"
#include "disk.h"
#include "fat32.h"
#include "printf.h"
#include "screen.h"
#include "string.h"

void file_tree(entry_t *entry, int depth);
void populate_files(entry_t *dir);

/**
 * @brief Entry point for the C code.
 */
void centry(uint8_t drive_number) {
    init_screen();
    init_disk(drive_number);
    init_fat();

    extern entry_t root;
    file_tree(&root, 0);

    const char *config_name = "/boot/iyasb.cfg";

    config_t config = {0};

    config_result_t result = read_config(config_name, &config);

    if (result != CONFIG_OK) {
        printf("Error: failed to read the config file (%s). Code 0x%02x\n", config_name, result);
        return;
    }

    printf("filename = %s\n", config.filename.value);
    printf("cmd-line = %s\n", config.cmd_line.value);
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
