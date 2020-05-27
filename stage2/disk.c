#include <disk.h>
#include <interrupts.h>
#include <stdint.h>

uint32_t bytes_per_sector;
uint8_t  saved_disk_number;

/**
 * @brief Read a single sector from the disk.
 *
 * @param sector Sector number (0-based)
 * @param buffer Memory location to write the data
 */
void read_sector(uint32_t sector, void *buffer) {
    static struct {
        uint8_t  size;
        uint8_t  zero;
        uint16_t readn;
        uint32_t buf;
        uint32_t start_lo;
        uint32_t start_hi;
    } __attribute__((packed)) dap;

    dap.size     = 0x10;
    dap.zero     = 0;
    dap.readn    = 1;
    dap.buf      = (uint32_t) buffer;
    dap.start_lo = sector;
    dap.start_hi = 0;

    gp_registers_t registers = {0};
    registers.ah             = 0x42;
    registers.dl             = saved_disk_number;
    registers.esi            = (uint32_t) &dap;

    real_int(0x13, registers);
}

/**
 * @brief Initiate disk operations on a disk.
 *
 * @param disk_number The BIOS disk number
 */
void init_disk(uint8_t disk_number) {
    saved_disk_number = disk_number;
    bytes_per_sector  = 512;
}
