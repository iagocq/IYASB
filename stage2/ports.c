#include <ports.h>

/**
 * @brief Read a byte from a port.
 * 
 * @param port The desired port to read from.
 * @return The value the port had.
 */
inline uint8_t port_byte_in(uint16_t port) {
    uint8_t result;
    asm volatile ("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

/**
 * @brief Write a byte into a port.
 * 
 * @param port The desired port to write to.
 * @param data The byte to be written.
 */
inline void port_byte_out(uint16_t port, uint8_t data) {
    asm volatile ("out %%al, %%dx" : : "a" (data), "d" (port));
}

/**
 * @brief Read a word from a port.
 * 
 * @param port The desired port to read from
 * @return The value the port had.
 */
inline uint16_t port_word_in(uint16_t port) {
    uint16_t result;
    asm volatile ("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

/**
 * @brief Write a word into a port.
 * 
 * @param port The desired port to write to.
 * @param data The word to be written
 */
inline void port_word_out(uint16_t port, uint16_t data) {
    asm volatile ("out %%ax, %%dx" : : "a" (data), "d" (port));
}
