#include "ports.h"
#include "printf.h"
#include "screen.h"
#include "string.h"
#include <stdint.h>

screen_t screen;

void            writechar(char chr, uint8_t col, uint8_t row);
void            clear_row(uint8_t row);
void            scroll_fb(uint8_t n);
uint16_t        get_cursor_offset();
void            set_cursor_offset(uint16_t offset);
void            set_cursor_pos(uint8_t col, uint8_t row);
inline uint16_t get_offset(uint8_t col, uint8_t row);
inline uint8_t  get_row_offset(uint16_t offset);
inline uint8_t  get_col_offset(uint16_t offset);
void *          memcpy(void *dst, const void *src, size_t n);

/**
 * @brief Initiates the screen to be usable later.
 * Create a wrapper around the physical video memory, setting values
 * like screen width, height and the pointer to the memory itself.
 * Values are hardcoded for now.
 */
void init_screen() {
    // hardcoding values for now
    screen.width = 80, screen.height = 25;
    screen.col = 0, screen.row = 24;

    screen.stride      = 2 * screen.width;
    screen.width_bytes = 2 * screen.width;

    screen.video_mem = (void *) 0xb8000;

    screen.default_char = 0x0e20;

    clear_screen();

    set_cursor_pos(screen.col, screen.row);
}

/**
 * @brief Clear the screen.
 * Write the default char into all of the screen, effectively cleaning it.
 *
 * @see screen.default_char
 */
void clear_screen() {
    // Iterate through each row
    for (uint8_t row = 0; row < screen.height; row++) {
        clear_row(row);
    }
}

/**
 * @brief Print a character and move the cursor forward.
 *
 * @param chr Character to print
 */
void putchar(const char chr) {
    switch (chr) {
        case '\n':
            screen.row++;
        case '\r':
            screen.col = 0;
            break;
        default:
            writechar(chr, screen.col, screen.row);
            screen.col++;
            break;
    }

    if (screen.col >= screen.width) {
        screen.col = 0;
        screen.row++;
    }

    if (screen.row >= screen.height) {
        scroll_fb(1);
        screen.row = screen.height - 1;
    }

    set_cursor_pos(screen.col, screen.row);
}

/**
 * @brief Print a string, followed by a new line.
 */
void puts(const char *str) {
    while (*str != '\0') {
        putchar(*str++);
    }
    putchar('\n');
}

/**
 * @brief Scroll the framebuffer n lines up.
 *
 * @param n The number of lines to scroll up
 */
void scroll_fb(const uint8_t n) {
    /* It is UB to memcpy overlapping source and destination memory regions.
       Even though this implementation does not cause UB in this case,
       I chose to remain compliant to the standards by copying one row at
       a time.
    */

    for (uint8_t row = n; row < screen.height; row++) {
        memcpy(screen.video_mem + (row - n) * screen.stride,
               screen.video_mem + row * screen.stride,
               screen.width_bytes);
    }

    for (uint8_t row = screen.height - n; row < screen.height; row++) {
        clear_row(row);
    }
}

/**
 * @brief Clear a row.
 * Write the default char into all of the row, effectively cleaning it.
 *
 * @param row The number of the row (0-based)
 */
void clear_row(const uint8_t row) {
    void *const row_mem = screen.video_mem + screen.stride * row;

    // Const pointer to the last character of this row
    uint16_t *const last_c_mem = row_mem + screen.width_bytes;

    // Iterate through the row, setting each character to default_char
    for (uint16_t *c_video_mem = row_mem; c_video_mem < last_c_mem; c_video_mem++) {
        *c_video_mem = screen.default_char;
    }
}

/**
 * @brief Write a char into the given position.
 * Doesn't check for special chars (like \\n or \\r).
 *
 * @param chr Character to write
 * @param col Column of the character
 * @param row Row of the character
 */
void writechar(const char chr, uint8_t col, uint8_t row) {
    uint16_t *video_mem = screen.video_mem + col * 2 + row * screen.stride;
    *video_mem          = (screen.default_char & 0xFF00) | ((uint8_t) chr);
}

/**
 * @brief Get cursor offset.
 */
uint16_t get_cursor_offset() {
    // Save the current index to restore before returning
    uint8_t last_index = port_byte_in(VGA_CRTC_REG_ADDR);

    // Select the Cursor Location High Register
    port_byte_out(VGA_CRTC_REG_ADDR, 0x0e);

    // Read the higher byte
    uint16_t offset = port_byte_in(VGA_CRTC_REG_DATA) << 8;

    // Select the Cursor Location Low Register
    port_byte_out(VGA_CRTC_REG_ADDR, 0x0f);
    offset |= port_byte_in(VGA_CRTC_REG_DATA);

    port_byte_out(VGA_CRTC_REG_ADDR, last_index);

    return offset;
}

/**
 * @brief Set cursor offset.
 */
void set_cursor_offset(uint16_t offset) {
    // Using the same logic as get_cursor_offset
    // except now there are writes instead of reads
    uint8_t last_index = port_byte_in(VGA_CRTC_REG_ADDR);

    port_byte_out(VGA_CRTC_REG_ADDR, 0x0e);
    port_byte_out(VGA_CRTC_REG_DATA, offset >> 8);

    port_byte_out(VGA_CRTC_REG_ADDR, 0x0f);
    port_byte_out(VGA_CRTC_REG_DATA, (uint8_t)(offset & 0xFF));

    port_byte_out(VGA_CRTC_REG_ADDR, last_index);
}

/**
 * @brief Set cursor position.
 *
 * @param col New cursor column
 * @param row New cursor row
 */
void set_cursor_pos(uint8_t col, uint8_t row) {
    set_cursor_offset(get_offset(col, row));
}

/**
 * @brief Calculate offset given column and row numbers.
 * Used to pass cursor position information to VGA hardware.
 *
 * @param col Column number
 * @param row Row number
 * @return Cursor offset
 */
uint16_t get_offset(uint8_t col, uint8_t row) {
    return row * screen.width + col;
}

/**
 * @brief Extract the row number from an offset.
 *
 * @param offset Cursor offset
 * @return Row number
 */
uint8_t get_row_offset(uint16_t offset) {
    return offset / screen.width;
}

/**
 * @brief Extract the column number from an offset.
 *
 * @param offset Cursor offset
 * @return Column number
 */
uint8_t get_col_offset(uint16_t offset) {
    return offset % screen.width;
}

void memdump(void *mem, size_t len, size_t bytes_per_line) {
    char     buf[11];
    size_t   max_offset_len = strlen(itoa(((size_t) mem) + len, buf, 16));
    uint8_t *umem           = (uint8_t *) mem;
    for (size_t i = 0; i < len; i++) {
        if ((i % bytes_per_line) == 0) {
            if (i != 0) {
                printf("  ");
                for (size_t j = i - bytes_per_line; j < i; j++) {
                    if (umem[j] >= 0x20 && umem[j] <= 0x7E)
                        putchar(umem[j]);
                    else
                        putchar('.');
                }
                putchar('\n');
            }
            printf("%0*x:", max_offset_len, &umem[i]);
        }
        if ((i % 2) == 0) {
            putchar(' ');
        }
        printf("%02x", umem[i]);
    }
    printf("  ");
    int s = (int) len - (int) bytes_per_line;
    for (size_t j = s < 0 ? 0 : s; j < len; j++) {
        if (umem[j] >= 0x20 && umem[j] <= 0x7E)
            putchar(umem[j]);
        else
            putchar('.');
    }
    putchar('\n');
}
