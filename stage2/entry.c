#include "types.h"

NORETURN void centry();

NORETURN void centry() {
    uint16_t *video = (uint16_t *) 0xb8000;
    *video = 0x7041;

    asm volatile ("cli");

    while (1) {
        asm volatile ("hlt");
    }
}
