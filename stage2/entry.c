#include <screen.h>

void centry() __attribute__((noreturn));

/**
 * @brief Entry point for the C code.
 */
void centry() {
    asm volatile ("cli");

    init_screen();

    puts("Hello, World!");

    while (1) {
        asm volatile ("hlt");
    }
}
