#include <disk.h>
#include <fat32.h>
#include <screen.h>

/**
 * @brief Entry point for the C code.
 */
void centry(uint8_t drive_number) {
    init_screen();
    init_disk(drive_number);
    init_fat();

    if (fopen("/stage2", "rb") == NULL) {
        puts("Failed to open file1");
    }

    if (fopen("/hi.txt", "rb") == NULL) {
        puts("Failed to open file2");
    }

    if (fopen("/folder1/file", "rb") == NULL) {
        puts("Failed to open file3");
    }

    puts("Hello, World!");
}
