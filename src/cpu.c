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

static void adc_a_r8(struct CPU *cpu, uint8_t *reg)
{
    cpu->registers.a += *reg + cpu->registers.f.c;

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

static void add_a_r8(struct CPU *cpu, uint8_t *reg)
{
    cpu->registers.a += *reg;

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

static void add_hl_r16(struct CPU *cpu, uint16_t *reg)
{
    cpu->registers.hl += *reg;

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

static void and_a_r8(struct CPU *cpu, uint8_t *reg)
{
    cpu->registers.a &= *reg;

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
static void bit_r8(struct CPU *cpu, uint8_t *reg)
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
static void call_cc_n16(struct CPU *cpu, uint8_t condition)
{
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

static void cp_a_r8(struct CPU *cpu, uint8_t *reg)
{
    int8_t result = cpu->registers.a - *reg;

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

// TODO
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
    uint8_t value = read_byte(cpu, cpu->registers.hl) - 1;
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
static void jp_cc_n16(struct CPU *cpu, uint8_t condition)
{
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
static void jr_n8(struct CPU *cpu)
{
    // Relative Jump to address n16.

    cpu->cycles += 3;
}

// TODO
static void jr_cc_n8(struct CPU *cpu, uint8_t condition)
{
    if (condition) {
        cpu->cycles += 3;
    } else {
        cpu->cycles += 2;
    }
}

static void ld_r8_r8(struct CPU *cpu, uint8_t *to, uint8_t *from)
{
    *to = *from;

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

static void ld_hl_r8(struct CPU *cpu, uint8_t *reg)
{
    write_byte(cpu, cpu->registers.hl, *reg);

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

static void ld_r16_a(struct CPU *cpu, uint16_t *reg)
{
    write_byte(cpu, *reg, cpu->registers.a);

    cpu->cycles += 2;
}

static void ld_n16_a(struct CPU *cpu)
{
    uint16_t address = read_word(cpu, cpu->registers.pc);
    cpu->registers.pc += 2;
    write_byte(cpu, address, cpu->registers.a);

    cpu->cycles += 4;
}

static void ld_a_r16(struct CPU *cpu, uint16_t *reg)
{
    cpu->registers.a = read_byte(cpu, *reg);

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

static void or_a_r8(struct CPU *cpu, uint8_t *reg)
{
    cpu->registers.a |= *reg;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0;

    cpu->cycles += 1;
}

static void or_a_hl(struct CPU *cpu)
{
    cpu->registers.a |= read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0;

    cpu->cycles += 2;
}

static void or_a_n8(struct CPU *cpu)
{
    cpu->registers.a |= read_byte(cpu, cpu->registers.pc);
    cpu->registers.pc += 1;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0;

    cpu->cycles += 2;
}

// TODO
static void pop_af(struct CPU *cpu)
{
    cpu->registers.af = read_word(cpu, cpu->registers.sp);
    cpu->registers.sp += 2;

    cpu->registers.f.z = 0; // TODO Set from bit 7 of the popped low byte.
    cpu->registers.f.n = 0; // TODO Set from bit 6 of the popped low byte.
    cpu->registers.f.h = 0; // TODO Set from bit 5 of the popped low byte.
    cpu->registers.f.c = 0; // TODO Set from bit 4 of the popped low byte.

    cpu->cycles += 3;
}

static void pop_r16(struct CPU *cpu, uint16_t *reg)
{
    *reg = read_word(cpu, cpu->registers.sp);
    cpu->registers.sp += 2;

    cpu->cycles += 3;
}

static void push_af(struct CPU *cpu)
{
    write_byte(cpu, cpu->registers.sp, cpu->registers.a);
    cpu->registers.sp -= 1;
    write_byte(cpu, cpu->registers.sp,
               cpu->registers.f.z << 7 | cpu->registers.f.n << 6 |
                   cpu->registers.f.h << 5 | cpu->registers.f.c << 4);
    cpu->registers.sp -= 1;

    cpu->cycles += 4;
}

// TODO
static void push_r16(struct CPU *cpu, uint16_t *reg)
{
    // write_word();
    cpu->registers.sp -= 2;

    cpu->cycles += 4;
}

// TODO
static void res_u3_r8(struct CPU *cpu, uint8_t *reg)
{
    // Set bit u3 in register r8 to 0. Bit 0 is the rightmost one, bit 7 the
    // leftmost one.

    cpu->cycles += 2;
}

// TODO
static void res_u3_hl(struct CPU *cpu)
{
    // Set bit u3 in the byte pointed by HL to 0. Bit 0 is the rightmost one,
    // bit 7 the leftmost one.

    cpu->cycles += 4;
}

static void ret(struct CPU *cpu)
{
    cpu->registers.pc = read_word(cpu, cpu->registers.sp);
    cpu->registers.sp += 2;

    cpu->cycles += 4;
}

// TODO
static void ret_cc(struct CPU *cpu, uint8_t condition)
{
    if (condition) {
        cpu->cycles += 5;
    } else {
        cpu->cycles += 2;
    }
}

// TODO
static void reti(struct CPU *cpu)
{
    // Return from subroutine and enable interrupts. This is basically
    // equivalent to executing EI then RET, meaning that IME is set right after
    // this instruction.

    cpu->cycles += 4;
}

// TODO
static void rl_r8(struct CPU *cpu, uint8_t *reg)
{
    // Rotate bits in register r8 left, through the carry flag.

    cpu->registers.f.z = reg == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 2;
}

// TODO
static void rl_hl(struct CPU *cpu)
{
    // Rotate the byte pointed to by HL left, through the carry flag.
    uint8_t value = read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = value == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 4;
}

// TODO
static void rla(struct CPU *cpu)
{
    // Rotate register A left, through the carry flag.

    cpu->registers.f.z = 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 1;
}

// TODO
static void rlc_r8(struct CPU *cpu, uint8_t *reg)
{
    // Rotate register r8 left.

    cpu->registers.f.z = *reg == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 2;
}

// TODO
static void rlc_hl(struct CPU *cpu)
{
    // Rotate register r8 left.
    uint8_t value = read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = value == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 4;
}

// TODO
static void rlca(struct CPU *cpu)
{
    // Rotate register A left.

    cpu->registers.f.z = 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 1;
}

// TODO
static void rr_r8(struct CPU *cpu, uint8_t *reg)
{
    // Rotate register r8 right, through the carry flag.

    cpu->registers.f.z = *reg == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 2;
}

// TODO
static void rr_hl(struct CPU *cpu)
{
    // Rotate the byte pointed to by HL right, through the carry flag.
    uint8_t value = read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = value == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 4;
}

// TODO
static void rra(struct CPU *cpu)
{
    // Rotate register A right, through the carry flag.

    cpu->registers.f.z = 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 1;
}

// TODO
static void rrc_r8(struct CPU *cpu, uint8_t *reg)
{
    // Rotate register r8 right.

    cpu->registers.f.z = *reg == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 2;
}

// TODO
static void rrc_hl(struct CPU *cpu)
{
    // Rotate the byte pointed to by HL right.
    uint8_t value = read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = value == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 4;
}

// TODO
static void rrca(struct CPU *cpu)
{
    // Rotate register A right.

    cpu->registers.f.z = 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 1;
}

// TODO
static void rst_vec(struct CPU *cpu)
{
    // Call address vec. This is a shorter and faster equivalent to CALL for
    // suitable values of vec.

    cpu->cycles += 4;
}

static void sbc_a_r8(struct CPU *cpu, uint8_t *reg)
{
    cpu->registers.a -= (*reg + cpu->registers.f.c);

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 1;
    cpu->registers.f.h = 0; // TODO Set if borrow from bit 4.
    cpu->registers.f.c = 0; // TODO Set if borrow (i.e. if (r8 + carry) > A).

    cpu->cycles += 1;
}

static void sbc_a_hl(struct CPU *cpu)
{
    cpu->registers.a -=
        (read_byte(cpu, cpu->registers.hl) + cpu->registers.f.c);

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 1;
    cpu->registers.f.h = 0; // TODO Set if borrow from bit 4.
    cpu->registers.f.c = 0; // TODO Set if borrow (i.e. if (r8 + carry) > A).

    cpu->cycles += 2;
}

static void sbc_a_n8(struct CPU *cpu)
{
    cpu->registers.a -=
        (read_byte(cpu, cpu->registers.pc) + cpu->registers.f.c);
    cpu->registers.pc += 1;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 1;
    cpu->registers.f.h = 0; // TODO Set if borrow from bit 4.
    cpu->registers.f.c = 0; // TODO Set if borrow (i.e. if (r8 + carry) > A).

    cpu->cycles += 2;
}

static void scf(struct CPU *cpu)
{
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 1;

    cpu->cycles += 1;
}

// TODO
static void set_u3_r8(struct CPU *cpu)
{
    // Set bit u3 in register r8 to 1. Bit 0 is the rightmost one, bit 7 the
    // leftmost one.

    cpu->cycles += 2;
}

// TODO
static void set_u3_hl(struct CPU *cpu)
{
    // Set bit u3 in the byte pointed by HL to 1. Bit 0 is the rightmost one,
    // bit 7 the leftmost one.

    cpu->cycles += 4;
}

// TODO
static void sla_r8(struct CPU *cpu, uint8_t *reg)
{
    // Shift Left Arithmetically register r8.

    cpu->registers.f.z = *reg == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 2;
}

// TODO
static void sla_hl(struct CPU *cpu)
{
    // Shift Left Arithmetically the byte pointed to by HL.
    uint8_t value = read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = value == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 4;
}

// TODO
static void sra_r8(struct CPU *cpu, uint8_t *reg)
{
    // Shift Right Arithmetically register r8 (bit 7 of r8 is unchanged).

    cpu->registers.f.z = *reg == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 2;
}

// TODO
static void sra_hl(struct CPU *cpu)
{
    // Shift Right Arithmetically the byte pointed to by HL (bit 7 of the byte
    // pointed to by HL is unchanged).
    uint8_t value = read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = value == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 4;
}

// TODO
static void srl_r8(struct CPU *cpu, uint8_t *reg)
{
    // Shift Right Logically register r8.

    cpu->registers.f.z = *reg == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 2;
}

// TODO
static void srl_hl(struct CPU *cpu)
{
    // Shift Right Logically the byte pointed to by HL.
    uint8_t value = read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = value == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0; // TODO Set according to result.

    cpu->cycles += 4;
}

// TODO
static void stop(struct CPU *cpu)
{
    // https://gbdev.io/pandocs/Reducing_Power_Consumption.html#using-the-stop-instruction
}

static void sub_a_r8(struct CPU *cpu, uint8_t *reg)
{
    cpu->registers.a -= *reg;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 1;
    cpu->registers.f.h = 0; // TODO Set if borrow from bit 4.
    cpu->registers.f.c = 0; // TODO Set if borrow (i.e. if r8 > A).

    cpu->cycles += 1;
}

static void sub_a_hl(struct CPU *cpu)
{
    cpu->registers.a -= read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 1;
    cpu->registers.f.h = 0; // TODO Set if borrow from bit 4.
    cpu->registers.f.c = 0; // TODO Set if borrow (i.e. if r8 > A).

    cpu->cycles += 2;
}

static void sub_a_n8(struct CPU *cpu)
{
    cpu->registers.a -= read_byte(cpu, cpu->registers.pc);
    cpu->registers.pc += 1;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 1;
    cpu->registers.f.h = 0; // TODO Set if borrow from bit 4.
    cpu->registers.f.c = 0; // TODO Set if borrow (i.e. if r8 > A).

    cpu->cycles += 2;
}

static void swap_r8(struct CPU *cpu, uint8_t *reg)
{
    *reg = (*reg << 4) | (*reg >> 4);

    cpu->registers.f.z = *reg == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0;

    cpu->cycles += 2;
}

static void swap_hl(struct CPU *cpu)
{
    uint8_t value = read_byte(cpu, cpu->registers.hl);
    value = (value << 4) | (value >> 4);
    write_byte(cpu, cpu->registers.hl, value);

    cpu->registers.f.z = value == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0;

    cpu->cycles += 4;
}

static void xor_a_r8(struct CPU *cpu, uint8_t *reg)
{
    cpu->registers.a ^= *reg;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0;

    cpu->cycles += 1;
}

static void xor_a_hl(struct CPU *cpu)
{
    cpu->registers.a ^= read_byte(cpu, cpu->registers.hl);

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0;

    cpu->cycles += 2;
}

static void xor_a_n8(struct CPU *cpu)
{
    cpu->registers.a ^= read_byte(cpu, cpu->registers.pc);
    cpu->registers.pc += 1;

    cpu->registers.f.z = cpu->registers.a == 0;
    cpu->registers.f.n = 0;
    cpu->registers.f.h = 0;
    cpu->registers.f.c = 0;

    cpu->cycles += 2;
}

void cpu_execute(struct CPU *cpu)
{
    if (!cpu->cycles) {
        uint8_t opcode = read_byte(cpu, cpu->registers.pc);
        cpu->registers.pc++;

        switch (opcode) {
        case 0x00:
            nop(cpu);
            break;
        case 0x01:
            ld_r16_n16(cpu, &cpu->registers.bc);
            break;
        case 0x02:
            ld_r16_a(cpu, &cpu->registers.bc);
            break;
        case 0x03:
            inc_r16(cpu, &cpu->registers.bc);
            break;
        case 0x04:
            inc_r8(cpu, &cpu->registers.b);
            break;
        case 0x05:
            dec_r8(cpu, &cpu->registers.b);
            break;
        case 0x06:
            ld_r8_n8(cpu, &cpu->registers.b);
            break;
        case 0x07:
            rlca(cpu);
            break;
        case 0x08:
            ld_n16_sp(cpu);
            break;
        case 0x09:
            add_hl_r16(cpu, &cpu->registers.bc);
            break;
        case 0x0A:
            ld_a_r16(cpu, &cpu->registers.bc);
            break;
        case 0x0B:
            dec_r16(cpu, &cpu->registers.bc);
            break;
        case 0x0C:
            inc_r8(cpu, &cpu->registers.c);
            break;
        case 0x0D:
            dec_r8(cpu, &cpu->registers.c);
            break;
        case 0x0E:
            ld_r8_n8(cpu, &cpu->registers.c);
            break;
        case 0x0F:
            rrca(cpu);
            break;
        case 0x10:
            stop(cpu);
            break;
        case 0x11:
            ld_r16_n16(cpu, &cpu->registers.de);
            break;
        case 0x12:
            ld_r16_a(cpu, &cpu->registers.de);
            break;
        case 0x13:
            inc_r16(cpu, &cpu->registers.de);
            break;
        case 0x14:
            inc_r8(cpu, &cpu->registers.d);
            break;
        case 0x15:
            dec_r8(cpu, &cpu->registers.d);
            break;
        case 0x16:
            ld_r8_n8(cpu, &cpu->registers.d);
            break;
        case 0x17:
            rla(cpu);
            break;
        case 0x18:
            jr_n8(cpu);
            break;
        case 0x19:
            add_hl_r16(cpu, &cpu->registers.de);
            break;
        case 0x1A:
            ld_a_r16(cpu, &cpu->registers.de);
            break;
        case 0x1B:
            dec_r16(cpu, &cpu->registers.de);
            break;
        case 0x1C:
            inc_r8(cpu, &cpu->registers.e);
            break;
        case 0x1D:
            dec_r8(cpu, &cpu->registers.e);
            break;
        case 0x1E:
            ld_r8_n8(cpu, &cpu->registers.e);
            break;
        case 0x1F:
            rra(cpu);
            break;
        case 0x20:
            jr_cc_n8(cpu, cpu->registers.f.z == 0);
            break;
        case 0x21:
            ld_r16_n16(cpu, &cpu->registers.hl);
            break;
        case 0x22:
            ld_r16_n16(cpu, &cpu->registers.hl);
            break;
        case 0x23:
            ld_hli_a(cpu);
            break;
        case 0x24:
            inc_r8(cpu, &cpu->registers.h);
            break;
        case 0x25:
            dec_r8(cpu, &cpu->registers.h);
            break;
        case 0x26:
            ld_r8_n8(cpu, &cpu->registers.h);
            break;
        case 0x27:
            daa(cpu);
            break;
        case 0x28:
            jr_cc_n8(cpu, cpu->registers.f.z == 1);
            break;
        case 0x29:
            add_hl_r16(cpu, &cpu->registers.hl);
            break;
        case 0x2A:
            ld_a_hli(cpu);
            break;
        case 0x2B:
            dec_r16(cpu, &cpu->registers.hl);
            break;
        case 0x2C:
            inc_r8(cpu, &cpu->registers.l);
            break;
        case 0x2D:
            dec_r8(cpu, &cpu->registers.l);
            break;
        case 0x2E:
            ld_r8_n8(cpu, &cpu->registers.l);
            break;
        case 0x2F:
            cpl(cpu);
            break;
        case 0x30:
            jr_cc_n8(cpu, cpu->registers.f.c == 0);
            break;
        case 0x31:
            ld_sp_n16(cpu);
            break;
        case 0x32:
            ld_hld_a(cpu);
            break;
        case 0x33:
            inc_sp(cpu);
            break;
        case 0x34:
            inc_hl(cpu);
            break;
        case 0x35:
            dec_hl(cpu);
            break;
        case 0x36:
            ld_hl_n8(cpu);
            break;
        case 0x37:
            scf(cpu);
            break;
        case 0x38:
            jr_cc_n8(cpu, cpu->registers.f.c == 1);
            break;
        case 0x39:
            add_hl_sp(cpu);
            break;
        case 0x3A:
            ld_a_hld(cpu);
            break;
        case 0x3B:
            dec_sp(cpu);
            break;
        case 0x3C:
            inc_r8(cpu, &cpu->registers.a);
            break;
        case 0x3D:
            dec_r8(cpu, &cpu->registers.a);
            break;
        case 0x3E:
            ld_r8_n8(cpu, &cpu->registers.a);
            break;
        case 0x3F:
            ccf(cpu);
            break;
        case 0x40:
            ld_r8_r8(cpu, &cpu->registers.b, &cpu->registers.b);
            break;
        case 0x41:
            ld_r8_r8(cpu, &cpu->registers.b, &cpu->registers.c);
            break;
        case 0x42:
            ld_r8_r8(cpu, &cpu->registers.b, &cpu->registers.d);
            break;
        case 0x43:
            ld_r8_r8(cpu, &cpu->registers.b, &cpu->registers.e);
            break;
        case 0x44:
            ld_r8_r8(cpu, &cpu->registers.b, &cpu->registers.h);
            break;
        case 0x45:
            ld_r8_r8(cpu, &cpu->registers.b, &cpu->registers.l);
            break;
        case 0x46:
            ld_r8_hl(cpu, &cpu->registers.b);
            break;
        case 0x47:
            ld_r8_r8(cpu, &cpu->registers.b, &cpu->registers.a);
            break;
        case 0x48:
            ld_r8_r8(cpu, &cpu->registers.c, &cpu->registers.b);
            break;
        case 0x49:
            ld_r8_r8(cpu, &cpu->registers.c, &cpu->registers.c);
            break;
        case 0x4A:
            ld_r8_r8(cpu, &cpu->registers.c, &cpu->registers.d);
            break;
        case 0x4B:
            ld_r8_r8(cpu, &cpu->registers.c, &cpu->registers.e);
            break;
        case 0x4C:
            ld_r8_r8(cpu, &cpu->registers.c, &cpu->registers.h);
            break;
        case 0x4D:
            ld_r8_r8(cpu, &cpu->registers.c, &cpu->registers.l);
            break;
        case 0x4E:
            ld_r8_hl(cpu, &cpu->registers.c);
            break;
        case 0x4F:
            ld_r8_r8(cpu, &cpu->registers.c, &cpu->registers.a);
            break;
        case 0x50:
            ld_r8_r8(cpu, &cpu->registers.d, &cpu->registers.b);
            break;
        case 0x51:
            ld_r8_r8(cpu, &cpu->registers.d, &cpu->registers.c);
            break;
        case 0x52:
            ld_r8_r8(cpu, &cpu->registers.d, &cpu->registers.d);
            break;
        case 0x53:
            ld_r8_r8(cpu, &cpu->registers.d, &cpu->registers.e);
            break;
        case 0x54:
            ld_r8_r8(cpu, &cpu->registers.d, &cpu->registers.h);
            break;
        case 0x55:
            ld_r8_r8(cpu, &cpu->registers.d, &cpu->registers.l);
            break;
        case 0x56:
            ld_r8_hl(cpu, &cpu->registers.d);
            break;
        case 0x57:
            ld_r8_r8(cpu, &cpu->registers.d, &cpu->registers.a);
            break;
        case 0x58:
            ld_r8_r8(cpu, &cpu->registers.e, &cpu->registers.b);
            break;
        case 0x59:
            ld_r8_r8(cpu, &cpu->registers.e, &cpu->registers.c);
            break;
        case 0x5A:
            ld_r8_r8(cpu, &cpu->registers.e, &cpu->registers.d);
            break;
        case 0x5B:
            ld_r8_r8(cpu, &cpu->registers.e, &cpu->registers.e);
            break;
        case 0x5C:
            ld_r8_r8(cpu, &cpu->registers.e, &cpu->registers.h);
            break;
        case 0x5D:
            ld_r8_r8(cpu, &cpu->registers.e, &cpu->registers.l);
            break;
        case 0x5E:
            ld_r8_hl(cpu, &cpu->registers.e);
            break;
        case 0x5F:
            ld_r8_r8(cpu, &cpu->registers.e, &cpu->registers.a);
            break;
        case 0x60:
            ld_r8_r8(cpu, &cpu->registers.h, &cpu->registers.b);
            break;
        case 0x61:
            ld_r8_r8(cpu, &cpu->registers.h, &cpu->registers.c);
            break;
        case 0x62:
            ld_r8_r8(cpu, &cpu->registers.h, &cpu->registers.d);
            break;
        case 0x63:
            ld_r8_r8(cpu, &cpu->registers.h, &cpu->registers.e);
            break;
        case 0x64:
            ld_r8_r8(cpu, &cpu->registers.h, &cpu->registers.h);
            break;
        case 0x65:
            ld_r8_r8(cpu, &cpu->registers.h, &cpu->registers.l);
            break;
        case 0x66:
            ld_r8_hl(cpu, &cpu->registers.h);
            break;
        case 0x67:
            ld_r8_r8(cpu, &cpu->registers.h, &cpu->registers.a);
            break;
        case 0x68:
            ld_r8_r8(cpu, &cpu->registers.l, &cpu->registers.b);
            break;
        case 0x69:
            ld_r8_r8(cpu, &cpu->registers.l, &cpu->registers.c);
            break;
        case 0x6A:
            ld_r8_r8(cpu, &cpu->registers.l, &cpu->registers.d);
            break;
        case 0x6B:
            ld_r8_r8(cpu, &cpu->registers.l, &cpu->registers.e);
            break;
        case 0x6C:
            ld_r8_r8(cpu, &cpu->registers.l, &cpu->registers.h);
            break;
        case 0x6D:
            ld_r8_r8(cpu, &cpu->registers.l, &cpu->registers.l);
            break;
        case 0x6E:
            ld_r8_hl(cpu, &cpu->registers.l);
            break;
        case 0x6F:
            ld_r8_r8(cpu, &cpu->registers.l, &cpu->registers.a);
            break;
        case 0x70:
            ld_hl_r8(cpu, &cpu->registers.b);
            break;
        case 0x71:
            ld_hl_r8(cpu, &cpu->registers.c);
            break;
        case 0x72:
            ld_hl_r8(cpu, &cpu->registers.d);
            break;
        case 0x73:
            ld_hl_r8(cpu, &cpu->registers.e);
            break;
        case 0x74:
            ld_hl_r8(cpu, &cpu->registers.h);
            break;
        case 0x75:
            ld_hl_r8(cpu, &cpu->registers.l);
            break;
        case 0x76:
            halt(cpu);
            break;
        case 0x77:
            ld_hl_r8(cpu, &cpu->registers.a);
            break;
        case 0x78:
            ld_r8_r8(cpu, &cpu->registers.a, &cpu->registers.b);
            break;
        case 0x79:
            ld_r8_r8(cpu, &cpu->registers.a, &cpu->registers.c);
            break;
        case 0x7A:
            ld_r8_r8(cpu, &cpu->registers.a, &cpu->registers.d);
            break;
        case 0x7B:
            ld_r8_r8(cpu, &cpu->registers.a, &cpu->registers.e);
            break;
        case 0x7C:
            ld_r8_r8(cpu, &cpu->registers.a, &cpu->registers.h);
            break;
        case 0x7D:
            ld_r8_r8(cpu, &cpu->registers.a, &cpu->registers.l);
            break;
        case 0x7E:
            ld_hl_r8(cpu, &cpu->registers.a);
            break;
        case 0x7F:
            ld_r8_r8(cpu, &cpu->registers.a, &cpu->registers.a);
            break;
        case 0x80:
            add_a_r8(cpu, &cpu->registers.b);
            break;
        case 0x81:
            add_a_r8(cpu, &cpu->registers.c);
            break;
        case 0x82:
            add_a_r8(cpu, &cpu->registers.d);
            break;
        case 0x83:
            add_a_r8(cpu, &cpu->registers.e);
            break;
        case 0x84:
            add_a_r8(cpu, &cpu->registers.h);
            break;
        case 0x85:
            add_a_r8(cpu, &cpu->registers.l);
            break;
        case 0x86:
            add_a_hl(cpu);
            break;
        case 0x87:
            add_a_r8(cpu, &cpu->registers.a);
            break;
        case 0x88:
            adc_a_r8(cpu, &cpu->registers.b);
            break;
        case 0x89:
            adc_a_r8(cpu, &cpu->registers.c);
            break;
        case 0x8A:
            adc_a_r8(cpu, &cpu->registers.d);
            break;
        case 0x8B:
            adc_a_r8(cpu, &cpu->registers.e);
            break;
        case 0x8C:
            adc_a_r8(cpu, &cpu->registers.h);
            break;
        case 0x8D:
            adc_a_r8(cpu, &cpu->registers.l);
            break;
        case 0x8E:
            adc_a_hl(cpu);
            break;
        case 0x8F:
            adc_a_r8(cpu, &cpu->registers.a);
            break;
        case 0x90:
            sub_a_r8(cpu, &cpu->registers.b);
            break;
        case 0x91:
            sub_a_r8(cpu, &cpu->registers.c);
            break;
        case 0x92:
            sub_a_r8(cpu, &cpu->registers.d);
            break;
        case 0x93:
            sub_a_r8(cpu, &cpu->registers.e);
            break;
        case 0x94:
            sub_a_r8(cpu, &cpu->registers.h);
            break;
        case 0x95:
            sub_a_r8(cpu, &cpu->registers.l);
            break;
        case 0x96:
            sub_a_hl(cpu);
            break;
        case 0x97:
            sub_a_r8(cpu, &cpu->registers.a);
            break;
        case 0x98:
            sbc_a_r8(cpu, &cpu->registers.b);
            break;
        case 0x99:
            sbc_a_r8(cpu, &cpu->registers.c);
            break;
        case 0x9A:
            sbc_a_r8(cpu, &cpu->registers.d);
            break;
        case 0x9B:
            sbc_a_r8(cpu, &cpu->registers.e);
            break;
        case 0x9C:
            sbc_a_r8(cpu, &cpu->registers.h);
            break;
        case 0x9D:
            sbc_a_r8(cpu, &cpu->registers.l);
            break;
        case 0x9E:
            sbc_a_hl(cpu);
            break;
        case 0x9F:
            sbc_a_r8(cpu, &cpu->registers.a);
            break;
        case 0xA0:
            and_a_r8(cpu, &cpu->registers.b);
            break;
        case 0xA1:
            and_a_r8(cpu, &cpu->registers.c);
            break;
        case 0xA2:
            and_a_r8(cpu, &cpu->registers.d);
            break;
        case 0xA3:
            and_a_r8(cpu, &cpu->registers.e);
            break;
        case 0xA4:
            and_a_r8(cpu, &cpu->registers.h);
            break;
        case 0xA5:
            and_a_r8(cpu, &cpu->registers.l);
            break;
        case 0xA6:
            and_a_hl(cpu);
            break;
        case 0xA7:
            and_a_r8(cpu, &cpu->registers.a);
            break;
        case 0xA8:
            xor_a_r8(cpu, &cpu->registers.b);
            break;
        case 0xA9:
            xor_a_r8(cpu, &cpu->registers.c);
            break;
        case 0xAA:
            xor_a_r8(cpu, &cpu->registers.d);
            break;
        case 0xAB:
            xor_a_r8(cpu, &cpu->registers.e);
            break;
        case 0xAC:
            xor_a_r8(cpu, &cpu->registers.h);
            break;
        case 0xAD:
            xor_a_r8(cpu, &cpu->registers.l);
            break;
        case 0xAE:
            xor_a_hl(cpu);
            break;
        case 0xAF:
            xor_a_r8(cpu, &cpu->registers.a);
            break;
        case 0xB0:
            or_a_r8(cpu, &cpu->registers.b);
            break;
        case 0xB1:
            or_a_r8(cpu, &cpu->registers.c);
            break;
        case 0xB2:
            or_a_r8(cpu, &cpu->registers.d);
            break;
        case 0xB3:
            or_a_r8(cpu, &cpu->registers.e);
            break;
        case 0xB4:
            or_a_r8(cpu, &cpu->registers.h);
            break;
        case 0xB5:
            or_a_r8(cpu, &cpu->registers.l);
            break;
        case 0xB6:
            or_a_hl(cpu);
            break;
        case 0xB7:
            or_a_r8(cpu, &cpu->registers.a);
            break;
        case 0xB8:
            cp_a_r8(cpu, &cpu->registers.b);
            break;
        case 0xB9:
            cp_a_r8(cpu, &cpu->registers.c);
            break;
        case 0xBA:
            cp_a_r8(cpu, &cpu->registers.d);
            break;
        case 0xBB:
            cp_a_r8(cpu, &cpu->registers.e);
            break;
        case 0xBC:
            cp_a_r8(cpu, &cpu->registers.h);
            break;
        case 0xBD:
            cp_a_r8(cpu, &cpu->registers.l);
            break;
        case 0xBE:
            cp_a_hl(cpu);
            break;
        case 0xBF:
            cp_a_r8(cpu, &cpu->registers.a);
            break;
        case 0xC0:
            ret_cc(cpu, cpu->registers.f.z == 0);
            break;
        case 0xC1:
            pop_r16(cpu, &cpu->registers.bc);
            break;
        case 0xC2:
            jp_cc_n16(cpu, cpu->registers.f.z == 0);
            break;
        case 0xC3:
            jp_n16(cpu);
            break;
        case 0xC4:
            call_cc_n16(cpu, cpu->registers.f.z == 0);
            break;
        case 0xC5:
            push_r16(cpu, &cpu->registers.bc);
            break;
        case 0xC6:
            add_a_n8(cpu);
            break;
        case 0xC7:
            rst_vec(cpu);
            break;
        case 0xC8:
            ret_cc(cpu, cpu->registers.f.z == 1);
            break;
        case 0xC9:
            ret(cpu);
            break;
        case 0xCA:
            jp_cc_n16(cpu, cpu->registers.f.z == 1);
            break;
        case 0xCC:
            call_cc_n16(cpu, cpu->registers.f.z == 1);
            break;
        case 0xCD:
            call_n16(cpu);
            break;
        case 0xCE:
            adc_a_n8(cpu);
            break;
        case 0xCF:
            rst_vec(cpu);
            break;
        case 0xD0:
            ret_cc(cpu, cpu->registers.f.c == 0);
            break;
        case 0xD1:
            pop_r16(cpu, &cpu->registers.de);
            break;
        case 0xD2:
            jp_cc_n16(cpu, cpu->registers.f.c == 0);
            break;
        case 0xD4:
            call_cc_n16(cpu, cpu->registers.f.c == 0);
            break;
        case 0xD5:
            push_r16(cpu, &cpu->registers.de);
            break;
        case 0xD6:
            sub_a_n8(cpu);
            break;
        case 0xD7:
            rst_vec(cpu);
            break;
        case 0xD8:
            ret_cc(cpu, cpu->registers.f.c == 1);
            break;
        case 0xD9:
            reti(cpu);
            break;
        case 0xDA:
            jp_cc_n16(cpu, cpu->registers.f.c == 1);
            break;
        case 0xDC:
            call_cc_n16(cpu, cpu->registers.f.c == 1);
            break;
        case 0xDE:
            sbc_a_n8(cpu);
            break;
        case 0xDF:
            rst_vec(cpu);
            break;
        case 0xE0:
            // TODO ld_n8_a(cpu);
            break;
        case 0xE1:
            pop_r16(cpu, &cpu->registers.hl);
            break;
        case 0xE2:
            // TODO ld_c_a(cpu);
            break;
        case 0xE5:
            push_r16(cpu, &cpu->registers.hl);
            break;
        case 0xE6:
            and_a_n8(cpu);
            break;
        case 0xE7:
            rst_vec(cpu);
            break;
        case 0xE8:
            add_sp_e8(cpu);
            break;
        case 0xE9:
            jp_hl(cpu);
            break;
        case 0xEA:
            ld_n16_a(cpu);
            break;
        case 0xEE:
            xor_a_n8(cpu);
            break;
        case 0xEF:
            rst_vec(cpu);
            break;
        case 0xF0:
            // TODO ld_a_n8(cpu);
            break;
        case 0xF1:
            pop_af(cpu);
            break;
        case 0xF2:
            // TODO ld_a_c(cpu);
            break;
        case 0xF3:
            di(cpu);
            break;
        case 0xF5:
            push_af(cpu);
            break;
        case 0xF6:
            or_a_n8(cpu);
            break;
        case 0xF7:
            rst_vec(cpu);
            break;
        case 0xF8:
            ld_hl_sp_e8(cpu);
            break;
        case 0xF9:
            ld_sp_hl(cpu);
            break;
        case 0xFA:
            ld_a_n16(cpu);
            break;
        case 0xFB:
            ei(cpu);
            break;
        case 0xFE:
            cp_a_n8(cpu);
            break;
        case 0xFF:
            rst_vec(cpu);
            break;
        default:
            printf("uknown opcode 0x%02x\n", opcode);
            break;
        }
    }

    cpu->cycles--;
}
