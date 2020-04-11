#pragma once

#include <stdint.h>
#include <stddef.h>

#define VGA_CRTC_REG_ADDR 0x3D4
#define VGA_CRTC_REG_DATA 0x3D5

typedef struct screen {
    /** Number of characters in a row */
    uint8_t width;
    /** Number of rows in the screen */
    uint8_t height;

    /** The current column the cursor is in, starting at 0 */
    uint8_t col;
    /** The current row the cursor is in, starting at 0 */
    uint8_t row;

    /** The number of bytes between the start and end of a row and the next */
    ptrdiff_t stride;

    /** The number of bytes between the first and last characters of a row */
    ptrdiff_t width_bytes;

    /** Default character and attribute to use while cleaning the screen */
    uint16_t default_char; 

    /** Pointer to the start of the video memory to use */
    void *video_mem;
} screen_t;

void init_screen();
void clear_screen();
void putchar(char chr);
void puts(const char *str);
