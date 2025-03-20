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

static void write_byte(struct CPU *cpu, uint16_t address, uint8_t value)
{
    memory[address] = value;
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

static void adc_a_r8(struct CPU *cpu, uint8_t reg)
{
    cpu->registers.a += reg + cpu->registers.f.c;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0; // TODO - Set if overflow from bit 3.
    cpu->registers.f.c = 0; // TODO - Set if overflow from bit 7.

    cpu->cycles += 1;
}

static void adc_a_hl(struct CPU *cpu)
{
    cpu->registers.a += read_byte(cpu, cpu->registers.hl) + cpu->registers.f.c;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0; // TODO - Set if overflow from bit 3.
    cpu->registers.f.c = 0; // TODO - Set if overflow from bit 7.

    cpu->cycles += 2;
}

static void adc_a_n8(struct CPU *cpu)
{
    cpu->registers.a += read_byte(cpu, cpu->registers.pc) + cpu->registers.f.c;
    cpu->registers.pc += 1;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0; // TODO - Set if overflow from bit 3.
    cpu->registers.f.c = 0; // TODO - Set if overflow from bit 7.

    cpu->cycles += 2;
}

static void add_a_r8(struct CPU *cpu, uint8_t reg)
{
    cpu->registers.a += reg;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0; // TODO - Set if overflow from bit 3.
    cpu->registers.f.c = 0; // TODO - Set if overflow from bit 7.

    cpu->cycles += 1;
}

static void add_a_hl(struct CPU *cpu)
{
    cpu->registers.a += read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0; // TODO - Set if overflow from bit 3.
    cpu->registers.f.c = 0; // TODO - Set if overflow from bit 7.

    cpu->cycles += 2;
}

static void add_a_n8(struct CPU *cpu)
{
    cpu->registers.a += read_byte(cpu, cpu->registers.pc);
    cpu->registers.pc += 1;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0; // TODO - Set if overflow from bit 3.
    cpu->registers.f.c = 0; // TODO - Set if overflow from bit 7.

    cpu->cycles += 2;
}

static void add_hl_r16(struct CPU *cpu, uint16_t reg)
{
    cpu->registers.hl += reg;

    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0; // TODO - Set if overflow from bit 11.
    cpu->registers.f.c = 0; // TODO - Set if overflow from bit 15.

    cpu->cycles += 2;
}

static void add_hl_sp(struct CPU *cpu)
{
    cpu->registers.hl += cpu->registers.sp;

    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0; // TODO - Set if overflow from bit 11.
    cpu->registers.f.c = 0; // TODO - Set if overflow from bit 15.

    cpu->cycles += 2;
}

static void add_sp_e8(struct CPU *cpu)
{
    // TODO check if works
    cpu->registers.sp += (int8_t)read_byte(cpu, cpu->registers.pc);
    cpu->registers.pc += 1;

    cpu->registers.f.z = 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0; // TODO - Set if overflow from bit 3.
    cpu->registers.f.c = 0; // TODO - Set if overflow from bit 7.

    cpu->cycles += 4;
}

static void and_a_r8(struct CPU *cpu, uint8_t reg)
{
    cpu->registers.a &= reg;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 1;
    cpu->registers.f.c = 0;

    cpu->cycles += 1;
}

static void and_a_hl(struct CPU *cpu)
{
    cpu->registers.a &= read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 1;
    cpu->registers.f.c = 0;

    cpu->cycles += 2;
}

static void and_a_n8(struct CPU *cpu)
{
    cpu->registers.a &= read_byte(cpu, cpu->registers.pc);
    cpu->registers.pc += 1;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 1;
    cpu->registers.f.c = 0;

    cpu->cycles += 2;
}

// TODO
static void bit_r8(struct CPU *cpu, uint8_t reg)
{

    cpu->registers.f.z = 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 1;

    cpu->cycles += 2;
}

// TODO
static void bit_hl(struct CPU *cpu)
{
    read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 1;

    cpu->cycles += 3;
}

// TODO
static void call_n16(struct CPU *cpu)
{
    read_word(cpu, cpu->registers.pc);
    cpu->registers.pc += 2;

    cpu->cycles += 6;
}

// TODO
static void call_cc_n16(struct CPU *cpu)
{
    uint8_t condition;
    if (condition) {
        cpu->cycles += 6;
    } else {
        cpu->cycles += 3;
    }
}

static void ccf(struct CPU *cpu)
{
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = !cpu->registers.f.c;

    cpu->cycles += 1;
}

static void cp_a_r8(struct CPU *cpu, uint8_t reg)
{
    int8_t result = cpu->registers.a - reg;

    cpu->registers.f.z = result == 0;
    cpu->registers.f.n = 1;
    cpu->registers.f.h = 0; // TODO Set if borrow from bit 4.
    cpu->registers.f.c = result < 0;

    cpu->cycles += 1;
}

static void cp_a_hl(struct CPU *cpu)
{
    int8_t result = cpu->registers.a - read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = result == 0;
    cpu->registers.f.n = 1;
    cpu->registers.f.h = 0; // TODO Set if borrow from bit 4.
    cpu->registers.f.c = result < 0;

    cpu->cycles += 2;
}

static void cp_a_n8(struct CPU *cpu)
{
    int8_t result = cpu->registers.a - read_byte(cpu, cpu->registers.pc);
    cpu->registers.pc += 1;

    cpu->registers.f.z = result == 0;
    cpu->registers.f.n = 1;
    cpu->registers.f.h = 0; // TODO Set if borrow from bit 4.
    cpu->registers.f.c = result < 0;

    cpu->cycles += 2;
}

static void cpl(struct CPU *cpu)
{
    cpu->registers.a = ~cpu->registers.a;

    cpu->registers.f.n = 1;
    cpu->registers.f.h = 1;

    cpu->cycles += 1;
}

static void daa(struct CPU *cpu)
{
    if (cpu->registers.f.n) {
    } else {
    }

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 1; // TODO Set or reset depending on the operation.

    cpu->cycles += 1;
}

static void dec_r8(struct CPU *cpu, uint8_t *reg)
{
    *reg -= 1;

    cpu->registers.f.z = *reg == 0;
    cpu->registers.f.n = 1;
    cpu->registers.f.h = 1; // TODO Set if borrow from bit 4.

    cpu->cycles += 1;
}

static void dec_hl(struct CPU *cpu)
{
    uint8_t value = read_byte(cpu, cpu->registers.hl);
    value -= 1;
    write_byte(cpu, cpu->registers.hl, value);

    cpu->registers.f.z = value == 0;
    cpu->registers.f.n = 1;
    cpu->registers.f.h = 1; // TODO Set if borrow from bit 4.

    cpu->cycles += 3;
}

static void dec_r16(struct CPU *cpu, uint16_t *reg)
{
    *reg -= 1;

    cpu->cycles += 2;
}

static void dec_sp(struct CPU *cpu)
{
    cpu->registers.sp -= 1;

    cpu->cycles += 2;
}

// TODO
static void di(struct CPU *cpu)
{
    // Disable Interrupts by clearing the IME flag.

    cpu->cycles += 1;
}

// TODO
static void ei(struct CPU *cpu)
{
    // Enable Interrupts by setting the IME flag.
    // The flag is only set after the instruction following EI.

    cpu->cycles += 1;
}

// TODO
static void halt(struct CPU *cpu)
{
    // Enter CPU low-power consumption mode until an interrupt occurs.
}

static void inc_r8(struct CPU *cpu, uint8_t *reg)
{
    *reg += 1;

    cpu->registers.f.z = *reg == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 1; // TODO Set if overflow from bit 3.

    cpu->cycles += 1;
}

static void inc_hl(struct CPU *cpu)
{
    uint8_t value = read_byte(cpu, cpu->registers.hl) + 1;
    write_byte(cpu, cpu->registers.hl, value);

    cpu->registers.f.z = value == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 1; // TODO Set if overflow from bit 3.

    cpu->cycles += 3;
}

static void inc_r16(struct CPU *cpu, uint16_t *reg)
{
    *reg += 1;

    cpu->cycles += 2;
}

static void inc_sp(struct CPU *cpu)
{
    cpu->registers.sp += 1;

    cpu->cycles += 2;
}

static void jp_n16(struct CPU *cpu)
{
    cpu->registers.pc = read_word(cpu, cpu->registers.pc);

    cpu->cycles += 4;
}

// TODO
static void jp_cc_n16(struct CPU *cpu)
{
    uint8_t condition;
    if (condition) {
        cpu->registers.pc = read_word(cpu, cpu->registers.pc);
        cpu->cycles += 4;
    } else {
        cpu->cycles += 3;
    }
}

static void jp_hl(struct CPU *cpu)
{
    cpu->registers.pc = cpu->registers.hl;

    cpu->cycles += 1;
}

// TODO
static void jr_n16(struct CPU *cpu)
{
    // Relative Jump to address n16.

    cpu->cycles += 3;
}

// TODO
static void jr_cc_n16(struct CPU *cpu)
{
    uint8_t condition;
    if (condition) {
        cpu->cycles += 3;
    } else {
        cpu->cycles += 2;
    }
}

static void ld_r8_r8(struct CPU *cpu, uint8_t *to, uint8_t from)
{
    *to = from;

    cpu->cycles += 1;
}

static void ld_r8_n8(struct CPU *cpu, uint8_t *reg)
{
    *reg = read_byte(cpu, cpu->registers.pc);
    cpu->registers.pc += 1;

    cpu->cycles += 2;
}

static void ld_r16_n16(struct CPU *cpu, uint16_t *reg)
{
    *reg = read_word(cpu, cpu->registers.pc);
    cpu->registers.pc += 2;

    cpu->cycles += 3;
}

static void ld_hl_r8(struct CPU *cpu, uint8_t reg)
{
    write_byte(cpu, cpu->registers.hl, reg);

    cpu->cycles += 2;
}

static void ld_hl_n8(struct CPU *cpu)
{
    uint8_t value = read_byte(cpu, cpu->registers.pc);
    cpu->registers.pc += 1;
    write_byte(cpu, cpu->registers.hl, value);

    cpu->cycles += 3;
}

static void ld_r8_hl(struct CPU *cpu, uint8_t *reg)
{
    *reg = read_byte(cpu, cpu->registers.hl);

    cpu->cycles += 2;
}

static void ld_r16_a(struct CPU *cpu, uint16_t reg)
{
    write_byte(cpu, reg, cpu->registers.a);

    cpu->cycles += 2;
}

static void ld_n16_a(struct CPU *cpu)
{
    uint16_t address = read_word(cpu, cpu->registers.pc);
    cpu->registers.pc += 2;
    write_byte(cpu, address, cpu->registers.a);

    cpu->cycles += 4;
}

static void ld_a_r16(struct CPU *cpu, uint16_t reg)
{
    cpu->registers.a = read_byte(cpu, reg);

    cpu->cycles += 2;
}

static void ld_a_n16(struct CPU *cpu)
{
    uint16_t address = read_word(cpu, cpu->registers.pc);
    cpu->registers.pc += 2;
    cpu->registers.a = read_byte(cpu, address);

    cpu->cycles += 4;
}

static void ld_hli_a(struct CPU *cpu)
{
    write_byte(cpu, cpu->registers.hl, cpu->registers.a);
    cpu->registers.hl += 1;

    cpu->cycles += 2;
}

static void ld_hld_a(struct CPU *cpu)
{
    write_byte(cpu, cpu->registers.hl, cpu->registers.a);
    cpu->registers.hl -= 1;

    cpu->cycles += 2;
}

static void ld_a_hld(struct CPU *cpu)
{
    cpu->registers.a = read_byte(cpu, cpu->registers.hl);
    cpu->registers.hl -= 1;

    cpu->cycles += 2;
}

static void ld_a_hli(struct CPU *cpu)
{
    cpu->registers.a = read_byte(cpu, cpu->registers.hl);
    cpu->registers.hl += 1;

    cpu->cycles += 2;
}

static void ld_sp_n16(struct CPU *cpu)
{
    cpu->registers.sp = read_word(cpu, cpu->registers.pc);
    cpu->registers.pc += 2;

    cpu->cycles += 3;
}

static void ld_n16_sp(struct CPU *cpu)
{
    uint16_t address = read_word(cpu, cpu->registers.pc);
    cpu->registers.pc += 2;
    write_byte(cpu, address, cpu->registers.sp & 0xFF);
    write_byte(cpu, address + 1, cpu->registers.sp >> 8);

    cpu->cycles += 5;
}

static void ld_hl_sp_e8(struct CPU *cpu)
{
    cpu->registers.hl =
        cpu->registers.sp + (int8_t)read_byte(cpu, cpu->registers.pc);
    cpu->registers.pc += 1;

    cpu->registers.f.z = 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0; // TODO - Set if overflow from bit 3.
    cpu->registers.f.c = 0; // TODO - Set if overflow from bit 7.

    cpu->cycles += 3;
}

static void ld_sp_hl(struct CPU *cpu)
{
    cpu->registers.sp = cpu->registers.hl;

    cpu->cycles += 2;
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
