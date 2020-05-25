#pragma once
#include <stdint.h>

#define xcat(a, b) a##b
#define cat(a, b)  xcat(a, b)
#define reg_def_32_16(reg)    \
    union {                   \
        uint32_t cat(e, reg); \
        uint16_t reg;         \
    }
#define reg_def_32_16_8(reg)                      \
    union {                                       \
        uint32_t cat(cat(e, reg), x);             \
        union {                                   \
            uint16_t cat(reg, x);                 \
            struct {                              \
                uint8_t cat(reg, l), cat(reg, h); \
            };                                    \
        };                                        \
    }

typedef struct gp_registers {
    reg_def_32_16(di);
    reg_def_32_16(si);

    reg_def_32_16(bp);
    reg_def_32_16(sp);

    reg_def_32_16_8(b);
    reg_def_32_16_8(d);
    reg_def_32_16_8(c);
    reg_def_32_16_8(a);
} gp_registers_t;

typedef struct int_registers {
    uint32_t       eflags;
    gp_registers_t gp_registers;
} int_registers_t;

int_registers_t real_int(uint8_t number, gp_registers_t registers);
