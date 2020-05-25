#include <interrupts.h>

extern int_registers_t *call_interrupt(uint8_t number, gp_registers_t register);

/**
 * @brief Run a real mode interrupt.
 *
 * @param number The number of the interrupt in the IVT
 * @param registers The values of the registers to use for the interrupt
 * @return int_registers_t The values of the general purpose registers and eflags after the
 * interrupt
 */
int_registers_t real_int(uint8_t number, gp_registers_t registers) {
    return *call_interrupt(number, registers);
}
