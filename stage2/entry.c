#include <disk.h>
#include <fat32.h>
#include <printf.h>
#include <screen.h>

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
}
