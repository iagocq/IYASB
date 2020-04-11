#include <screen.h>

void centry() __attribute__((noreturn));

/**
 * @brief Entry point for the C code.
 */
void centry() {
    asm volatile ("cli");

    init_screen();

    for (int i = 0; i < 81; i++) {
        putchar('a');
    }

    putchar('\n');

    for (int i = 0; i < 20; i++) {
        putchar('b');
    }
    putchar('\r');

    for (int i = 0; i < 5; i++) {
        putchar('c');
    }

    while (1) {
        asm volatile ("hlt");
    }
}
