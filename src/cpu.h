#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#define reg(name, high, low)                                                   \
    union {                                                                    \
        uint16_t name;                                                         \
        struct {                                                               \
            uint8_t high;                                                      \
            uint8_t low;                                                       \
        };                                                                     \
    };

struct Registers {
    union {
        uint16_t af;
        struct {
            uint8_t a;
            struct {
                uint8_t unused_1 : 1;
                uint8_t unused_2 : 1;
                uint8_t unused_3 : 1;
                uint8_t unused_4 : 1;
                uint8_t z : 1; // zero_flag
                uint8_t n : 1; // subtraction_flag
                uint8_t h : 1; // half_carry_flag
                uint8_t c : 1; // carry_flag
            } f;
        };
    };
    reg(bc, b, c);
    reg(de, d, e);
    reg(hl, h, l);
    uint16_t sp;
    uint16_t pc;
};

struct CPU {
    struct Registers registers;
    uint8_t cycles;
};

void cpu_reset(struct CPU *);
void cpu_execute(struct CPU *);

#endif // !CPU_H
