#include "cpu.h"
#include <stdio.h>

static uint8_t memory[16 * 0x1000];

static uint8_t read_byte(struct CPU *cpu, uint16_t address)
{
    return memory[address];
}

static uint16_t read_word(struct CPU *cpu, uint16_t address)
{
    // little endian

    uint8_t low = memory[address + 1];
    uint8_t high = memory[address];

    return (low << 8) | high;
}

void cpu_reset(struct CPU *cpu)
{
    cpu->registers.af = 0;
    cpu->registers.bc = 0;
    cpu->registers.de = 0;
    cpu->registers.hl = 0;
    cpu->registers.sp = 0;
    cpu->registers.pc = 0;

    cpu->cycles = 0;
}

static void nop(struct CPU *cpu) { cpu->cycles += 1; }

void cpu_execute(struct CPU *cpu)
{
    if (!cpu->cycles) {
        uint8_t opcode = read_byte(cpu, cpu->registers.pc);
        cpu->registers.pc++;

        switch (opcode) {
        case 0x00:
            nop(cpu);
            break;
        default:
            printf("uknown opcode 0x%02x\n", opcode);
            break;
        }
    }

    cpu->cycles--;
}
