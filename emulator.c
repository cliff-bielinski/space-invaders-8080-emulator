#include "emulator.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t port_in(i8080 *cpu, uint8_t port);
static void port_out(i8080 *cpu, uint8_t port, uint8_t value);

// Add Register or Memory to Accumulator with Carry
int
ADC(i8080 *cpu, const u_int8_t *reg)
{
  // get carry bit and add A, register and CY together
  uint8_t carry = ((cpu->flags & FLAG_CY) == FLAG_CY);
  uint16_t result = cpu->a + *reg + carry;

  // set A to sum and set flags
  update_aux_carry_flag(cpu, cpu->a, (*reg + carry));
  cpu->a = (uint8_t)result;
  update_zero_flag(cpu, cpu->a);
  update_sign_flag(cpu, cpu->a);
  update_parity_flag(cpu, cpu->a);
  update_carry_flag(cpu, result > MAX_8_BIT_VALUE);
  return 4; // NOLINT
}

// Logical AND with Accumulator
int
ANA(i8080 *cpu, const uint8_t value)
{
  cpu->a = cpu->a & value;
  update_zero_flag(cpu, cpu->a);
  update_sign_flag(cpu, cpu->a);
  update_parity_flag(cpu, cpu->a);
  update_carry_flag(cpu, false);
  return 4; // NOLINT
}

// Call Address
int
CALL(i8080 *cpu, uint16_t address)
{
  uint16_t return_address = readRegisterPair(cpu, PC) + 3; // NOLINT
  cpu_write_mem(cpu, cpu->sp - 1,
                (uint8_t)((return_address & UPPER_8_BIT_MASK) >> BYTE));
  cpu_write_mem(cpu, cpu->sp - 2,
                (uint8_t)(return_address & LOWER_8_BIT_MASK));
  cpu->sp -= 2;
  writeRegisterPair(cpu, PC, address);
  return 17; // NOLINT
}

// Compare Register or Memory with Accumulator
int
CMP(i8080 *cpu, uint8_t value)
{
  u_int8_t result = cpu->a - value;
  update_zero_flag(cpu, result);
  update_carry_flag(cpu, value > cpu->a);
  update_parity_flag(cpu, result);
  update_sign_flag(cpu, result);
  return 4; // NOLINT
}

// Decimal Adjust Accumulator
int
DAA(i8080 *cpu)
{
  // break accumulator into 2 4-bit pieces
  uint8_t lo_nibble = (cpu->a & LOWER_4_BIT_MASK);
  uint8_t hi_nibble = ((cpu->a & UPPER_4_BIT_MASK) >> NIBBLE);

  // STEP 1: if least sig bits are > 9 or AC is set, increment A by 6
  if (lo_nibble > 9 || ((cpu->flags & FLAG_AC) == FLAG_AC)) // NOLINT
    {
      cpu->a += 6; // NOLINT

      // set AC to 1
      cpu->flags |= FLAG_AC;

      // reassign nibbles to newly incremented A value
      lo_nibble = (cpu->a & LOWER_4_BIT_MASK);
      hi_nibble = ((cpu->a & UPPER_4_BIT_MASK) >> NIBBLE);
    }
  else
    {
      // clear AC if no carry
      cpu->flags &= ~FLAG_AC;
    }

  // STEP 2: if most sig bits are NOW > 9 or CY is set, increment hi_nibble by
  // 6
  if (hi_nibble > 9 || ((cpu->flags & FLAG_CY) == FLAG_CY)) // NOLINT
    {
      hi_nibble += 6; // NOLINT
      // reconstruct 8-bit A register after hi_nibble increment
      cpu->a = ((hi_nibble << NIBBLE) | lo_nibble);

      // set CY to 1
      update_carry_flag(cpu, true);
    }

  return 4; // NOLINT
}

// Double Add
int
DAD(i8080 *cpu, int pair)
{
  uint32_t result
      = (uint32_t)(readRegisterPair(cpu, pair) + readRegisterPair(cpu, HL));
  update_carry_flag(cpu, result > MAX_16_BIT_VALUE);
  writeRegisterPair(cpu, HL, (uint16_t)result);
  return 10; // NOLINT
}

// Add reg
int
add_reg_accum(i8080 *cpu, uint8_t value)
{
  uint16_t result = cpu->a + value;
  update_sign_flag(cpu, result);
  update_zero_flag(cpu, result);
  update_aux_carry_flag(cpu, cpu->a, value);
  update_parity_flag(cpu, result);
  if (result > MAX_8_BIT_VALUE)
    {
      update_carry_flag(cpu, true);
    }
  else
    {
      update_carry_flag(cpu, false);
    }
  cpu->a = (uint8_t)result;
  return 4; // NOLINT
}

// Decrement Register
int
DCR(i8080 *cpu, uint8_t *reg)
{
  update_aux_carry_flag(cpu, *reg, MAX_8_BIT_VALUE);
  *reg -= 1;
  update_zero_flag(cpu, *reg);
  update_sign_flag(cpu, *reg);
  update_parity_flag(cpu, *reg);
  return 5; // NOLINT
}

// Decrement Register Pair
int
DCX(i8080 *cpu, int pair)
{
  uint16_t value = readRegisterPair(cpu, pair) - 1;
  writeRegisterPair(cpu, pair, value);
  return 5; // NOLINT
}

// Increment Register
int
INR(i8080 *cpu, uint8_t *reg)
{
  update_aux_carry_flag(cpu, *reg, 0x01);
  *reg += 1;
  update_zero_flag(cpu, *reg);
  update_sign_flag(cpu, *reg);
  update_parity_flag(cpu, *reg);
  return 5; // NOLINT
}

// Increment Register Pair
int
INX(i8080 *cpu, int pair)
{
  uint16_t value = readRegisterPair(cpu, pair) + 1;
  writeRegisterPair(cpu, pair, value);
  return 5; // NOLINT
}

// Jump to Address
int
JMP(i8080 *cpu)
{
  writeRegisterPair(cpu, PC, getImmediate16BitValue(cpu));
  return 10; // NOLINT
}

// Load Accumulator
int
LDAX(i8080 *cpu, int pair)
{
  uint16_t address = readRegisterPair(cpu, pair);
  cpu->a = cpu_read_mem(cpu, address);
  return 7; // NOLINT
}

// Load to HL
int
LHLD(i8080 *cpu, u_int16_t address)
{
  cpu->l = cpu_read_mem(cpu, address);
  cpu->h = cpu_read_mem(cpu, (address + 1));
  return 16; // NOLINT
}

// Load 16-bit Data to Register Pair
int
LXI(i8080 *cpu, int pair, uint16_t value)
{
  writeRegisterPair(cpu, pair, value);
  return 10; // NOLINT
}

// Move register value
int
MOV(uint8_t *dest, const uint8_t *src)
{
  *dest = *src;
  return 5; // NOLINT
}

// Move register value to memory
int
MOV_TO_MEM(i8080 *cpu, const uint8_t *reg)
{
  cpu_write_mem(cpu, readRegisterPair(cpu, HL), *reg);
  return 7; // NOLINT
}

// Move memory to register
int
MOV_FROM_MEM(i8080 *cpu, uint8_t *reg)
{
  *reg = cpu_read_mem(cpu, readRegisterPair(cpu, HL));
  return 7; // NOLINT
}

// Move 8-bit Data to Register
int
MVI(uint8_t *reg, uint8_t value)
{
  *reg = value;
  return 7; // NOLINT
}

// No Operation
int
NOP()
{
  return 4; // NOLINT
}

// Logical OR with accumulator
int
ORA(i8080 *cpu, u_int8_t value)
{
  cpu->a |= value;
  // always set to 0
  update_carry_flag(cpu, false);
  update_parity_flag(cpu, cpu->a);
  update_sign_flag(cpu, cpu->a);
  update_zero_flag(cpu, cpu->a);
  return 4;
}

// Pop from Stack
int
POP(i8080 *cpu, int pair)
{
  uint16_t value = ((cpu_read_mem(cpu, cpu->sp + 1) << BYTE)
                    | cpu_read_mem(cpu, cpu->sp));
  writeRegisterPair(cpu, pair, value);
  cpu->sp += 2;
  return 10; // NOLINT
}

// Push to Stack
int
PUSH(i8080 *cpu, int pair)
{
  uint16_t value = readRegisterPair(cpu, pair);
  cpu_write_mem(cpu, cpu->sp - 1,
                (uint8_t)((value & UPPER_8_BIT_MASK) >> BYTE));
  cpu_write_mem(cpu, cpu->sp - 2, (uint8_t)(value & LOWER_8_BIT_MASK));
  cpu->sp -= 2;
  return 11; // NOLINT
}

// Return to Address
int
RET(i8080 *cpu)
{
  uint16_t address = ((cpu_read_mem(cpu, cpu->sp + 1) << BYTE)
                      | cpu_read_mem(cpu, cpu->sp));
  cpu->sp += 2;
  writeRegisterPair(cpu, PC, address);
  return 10; // NOLINT
}

// Subtract Immediate Value with Borrow
int
SBI(i8080 *cpu, u_int8_t value)
{
  // get carry bit and add A, register and CY together
  uint8_t carry = ((cpu->flags & FLAG_CY) == FLAG_CY);

  // set A to sum and set flags
  update_aux_carry_flag(cpu, cpu->a, (~value + carry));
  update_carry_flag(cpu, cpu->a < (value + carry));
  cpu->a = cpu->a - (value + carry);
  update_zero_flag(cpu, cpu->a);
  update_sign_flag(cpu, cpu->a);
  update_parity_flag(cpu, cpu->a);

  return 7; // NOLINT
}

// Store Accumulator
int
STAX(i8080 *cpu, int pair)
{
  uint16_t address = readRegisterPair(cpu, pair);
  cpu_write_mem(cpu, address, cpu->a);
  return 7; // NOLINT
}

// Subtract Register from Accumulator
int
SUB(i8080 *cpu, const uint8_t value)
{
  // set flags and subtract register value from accumulator
  update_aux_carry_flag(cpu, cpu->a, ((u_int8_t)~value + 1));
  update_carry_flag(cpu, cpu->a < value); // carry if borrow
  cpu->a = cpu->a - value;
  update_zero_flag(cpu, cpu->a);
  update_sign_flag(cpu, cpu->a);
  update_parity_flag(cpu, cpu->a);
  return 4; // NOLINT
}

// Logical XOR with Accumulator
int
XRA(i8080 *cpu, const uint8_t *reg)
{
  cpu->a = cpu->a ^ *reg;
  update_zero_flag(cpu, cpu->a);
  update_sign_flag(cpu, cpu->a);
  update_parity_flag(cpu, cpu->a);
  update_aux_carry_flag(cpu, cpu->a, MAX_8_BIT_VALUE);
  update_carry_flag(cpu, false);
  return 4; // NOLINT
}

// Get next byte in program counter
uint8_t
getImmediate8BitValue(i8080 *cpu)
{
  return cpu_read_mem(cpu, cpu->pc + 1);
}

// Get next two bytes in program counter
uint16_t
getImmediate16BitValue(i8080 *cpu)
{
  uint8_t lo = cpu_read_mem(cpu, cpu->pc + 1);
  uint8_t hi = cpu_read_mem(cpu, cpu->pc + 2);
  return (uint16_t)((hi << BYTE) | lo);
}

// Get 16-bit value for given register pair
uint16_t
readRegisterPair(i8080 *cpu, int pair)
{
  switch (pair)
    {
    case PSW:
      {
        return (uint16_t)((cpu->a << BYTE) | cpu->flags);
      }
    case BC:
      {
        return (uint16_t)((cpu->b << BYTE) | cpu->c);
      }
    case DE:
      {
        return (uint16_t)((cpu->d << BYTE) | cpu->e);
      }
    case HL:
      {
        return (uint16_t)((cpu->h << BYTE) | cpu->l);
      }
    case SP:
      {
        return cpu->sp;
      }
    case PC:
      {
        return cpu->pc;
      }
    default:
      {
        fprintf(stderr, "Invalid register pair: %d", pair);
        exit(EXIT_FAILURE);
      }
    }
}

// Write 16-bit value to given register pair
void
writeRegisterPair(i8080 *cpu, int pair, uint16_t value)
{
  switch (pair)
    {
    case PSW:
      {
        cpu->a = (uint8_t)((value & UPPER_8_BIT_MASK) >> BYTE);
        cpu->flags = (uint8_t)(value & LOWER_8_BIT_MASK);
        break;
      }
    case BC:
      {
        cpu->b = (uint8_t)((value & UPPER_8_BIT_MASK) >> BYTE);
        cpu->c = (uint8_t)(value & LOWER_8_BIT_MASK);
        break;
      }
    case DE:
      {
        cpu->d = (uint8_t)((value & UPPER_8_BIT_MASK) >> BYTE);
        cpu->e = (uint8_t)(value & LOWER_8_BIT_MASK);
        break;
      }
    case HL:
      {
        cpu->h = (uint8_t)((value & UPPER_8_BIT_MASK) >> BYTE);
        cpu->l = (uint8_t)(value & LOWER_8_BIT_MASK);
        break;
      }
    case SP:
      {
        cpu->sp = value;
        break;
      }
    case PC:
      {
        cpu->pc = value;
        break;
      }
    default:
      {
        fprintf(stderr, "Invalid register pair: %d", pair);
        exit(EXIT_FAILURE);
      }
    }
}

// Execute Instruction
int
execute_instruction(i8080 *cpu, uint8_t opcode)
{
  int num_cycles = 0;
  switch (opcode)
    {
    case 0x00: // NOLINT
      {        // NOP
        num_cycles = NOP();
        break;
      }
    case 0x01: // NOLINT
      {        // LXI B
        num_cycles = LXI(cpu, BC, getImmediate16BitValue(cpu));
        cpu->pc += 2;
        break;
      }
    case 0x02: // NOLINT
      {        // STAX B
        num_cycles = STAX(cpu, BC);
        break;
      }
    case 0x03: // NOLINT
      {        // INX B
        num_cycles = INX(cpu, BC);
        break;
      }
    case 0x04: // NOLINT
      {        // INR B
        num_cycles = INR(cpu, &cpu->b);
        break;
      }
    case 0x05: // NOLINT
      {        // DCR B
        num_cycles = DCR(cpu, &cpu->b);
        break;
      }
    case 0x06: // NOLINT
      {        // MVI B, mem8
        num_cycles = MVI(&cpu->b, getImmediate8BitValue(cpu));
        cpu->pc += 1;
        break;
      }
    case 0x07: // NOLINT
      {        // RLC
        // keep bit 7
        uint8_t tmp = cpu->a >> 7; // NOLINT

        // left shift register a
        cpu->a = cpu->a << 1;

        // replace wrapped bit
        cpu->a ^= tmp;

        // set cy flag to prev bit 7
        if (tmp != 0)
          {
            update_carry_flag(cpu, true);
          }
        else
          {
            update_carry_flag(cpu, false);
          }
        num_cycles = 4; // NOLINT
        break;
      }
    case 0x09: // NOLINT
      {        // DAD B
        num_cycles = DAD(cpu, BC);
        break;
      }
    case 0x0a: // NOLINT
      {        // LDAX B
        num_cycles = LDAX(cpu, BC);
        break;
      }
    case 0x0b: // NOLINT
      {        // DCX B
        num_cycles = DCX(cpu, BC);
        break;
      }
    case 0x0c: // NOLINT
      {        // INR C
        num_cycles = INR(cpu, &cpu->c);
        break;
      }
    case 0x0d: // NOLINT
      {        // DCR C
        num_cycles = DCR(cpu, &cpu->c);
        break;
      }
    case 0x0e: // NOLINT
      {        // MVI C, D8
        num_cycles = MVI(&cpu->c, getImmediate8BitValue(cpu));
        cpu->pc += 1;
        break;
      }
    case 0x0f: // NOLINT
      {        // RRC
        // keep bit 0
        uint8_t tmp = cpu->a << 7; // NOLINT

        // right shift register a
        cpu->a = cpu->a >> 1;

        // replace wrapped bit
        cpu->a ^= tmp;

        // set cy flag to prev bit 0
        if (tmp != 0)
          {
            update_carry_flag(cpu, true);
          }
        else
          {
            update_carry_flag(cpu, false);
          }
        num_cycles = 4; // NOLINT
        break;
      }
    case 0x11: // NOLINT
      {        // LXI D
        num_cycles = LXI(cpu, DE, getImmediate16BitValue(cpu));
        cpu->pc += 2;
        break;
      }
    case 0x12: // NOLINT
      {        // STAX D
        num_cycles = STAX(cpu, DE);
        break;
      }
    case 0x13: // NOLINT
      {        // INX D
        num_cycles = INX(cpu, DE);
        break;
      }
    case 0x14: // NOLINT
      {        // INR D
        num_cycles = INR(cpu, &cpu->d);
        break;
      }
    case 0x15: // NOLINT
      {        // DCR D
        num_cycles = DCR(cpu, &cpu->d);
        break;
      }
    case 0x16: // NOLINT
      {        // MVI D
        num_cycles = MVI(&cpu->d, getImmediate8BitValue(cpu));
        cpu->pc += 1;
        break;
      }
    case 0x19: // NOLINT
      {        // DAD D
        num_cycles = DAD(cpu, DE);
        break;
      }
    case 0x1a: // NOLINT
      {        // LDAX D
        num_cycles = LDAX(cpu, DE);
        break;
      }
    case 0x1b: // NOLINT
      {        // DCX D
        num_cycles = DCX(cpu, DE);
        break;
      }
    case 0x1c: // NOLINT
      {        // INR E
        num_cycles = INR(cpu, &cpu->e);
        break;
      }
    case 0x1d: // NOLINT
      {        // DCR E
        num_cycles = DCR(cpu, &cpu->e);
        break;
      }
    case 0x1e: // NOLINT
      {        // MVI E
        num_cycles = MVI(&cpu->e, getImmediate8BitValue(cpu));
        cpu->pc += 1;
        break;
      }
    case 0x1f: // NOLINT
      {        // RAR
        // keep old bit 0
        uint8_t bit0 = cpu->a & 0x01; // NOLINT

        // right shift register a by 1
        cpu->a = cpu->a >> 1;

        // replace bit 7 with old CY value
        if ((cpu->flags & FLAG_CY) != 0)
          {
            cpu->a |= 0x80; // NOLINT
          }

        // set new CY to previous bit 0
        if (bit0 != 0)
          {
            update_carry_flag(cpu, true);
          }
        else
          {
            update_carry_flag(cpu, false);
          }

        num_cycles = 4; // NOLINT
        break;
      }
    case 0x21: // NOLINT
      {        // LXI H
        num_cycles = LXI(cpu, HL, getImmediate16BitValue(cpu));
        cpu->pc += 2;
        break;
      }
    case 0x22: // NOLINT
      {        // SHLD addr
        uint16_t address = getImmediate16BitValue(cpu);
        cpu_write_mem(cpu, address, cpu->l);
        cpu_write_mem(cpu, (address + 1), cpu->h);
        num_cycles = 16; // NOLINT
        cpu->pc += 2;
        break;
      }
    case 0x23: // NOLINT
      {        // INX H
        num_cycles = INX(cpu, HL);
        break;
      }
    case 0x24: // NOLINT
      {        // INR H
        num_cycles = INR(cpu, &cpu->h);
        break;
      }
    case 0x25: // NOLINT
      {        // DCR H
        num_cycles = DCR(cpu, &cpu->h);
        break;
      }
    case 0x26: // NOLINT
      {        // MVI H, D8
        num_cycles = MVI(&cpu->h, getImmediate8BitValue(cpu));
        cpu->pc += 1;
        break;
      }
    case 0x27: // NOLINT
      {        // DAA
        num_cycles = DAA(cpu);
        break;
      }
    case 0x29: // NOLINT
      {        // DAD H
        num_cycles = DAD(cpu, HL);
        break;
      }
    case 0x2a: // NOLINT
      {        // LHLD
        num_cycles = LHLD(cpu, getImmediate16BitValue(cpu));
        cpu->pc += 2;
        break;
      }
    case 0x2b: // NOLINT
      {        // DCX H
        num_cycles = DCX(cpu, HL);
        break;
      }
    case 0x2c: // NOLINT
      {        // INR L
        num_cycles = INR(cpu, &cpu->l);
        break;
      }
    case 0x2e: // NOLINT
      {        // MVI L
        num_cycles = MVI(&cpu->l, getImmediate8BitValue(cpu));
        cpu->pc += 1;
        break;
      }
    case 0x2f: // NOLINT
      {        // CMA
        cpu->a = ~cpu->a;
        num_cycles = 4;
        break;
      }
    case 0x31: // NOLINT
      {        // LXI SP
        num_cycles = LXI(cpu, SP, getImmediate16BitValue(cpu));
        cpu->pc += 2;
        break;
      }
    case 0x32: // NOLINT
      {        // STA
        uint16_t address = getImmediate16BitValue(cpu);
        cpu_write_mem(cpu, address, cpu->a);
        cpu->pc += 2;
        num_cycles = 13; // NOLINT
        break;
      }
    case 0x34: // NOLINT
      {        // INR M
        uint16_t address = readRegisterPair(cpu, HL);
        uint8_t value = cpu_read_mem(cpu, address);
        update_aux_carry_flag(cpu, value, 0x01);
        value += 1;
        update_zero_flag(cpu, value);
        update_sign_flag(cpu, value);
        update_parity_flag(cpu, value);
        cpu_write_mem(cpu, address, value);
        num_cycles = 10; // NOLINT
        break;
      }
    case 0x35: // NOLINT
      {        // DCR M
        uint16_t address = readRegisterPair(cpu, HL);
        uint8_t mem_value = cpu_read_mem(cpu, address);
        uint8_t result = mem_value - 1;
        update_zero_flag(cpu, result);
        update_sign_flag(cpu, result);
        update_parity_flag(cpu, result);
        update_aux_carry_flag(cpu, mem_value, MAX_8_BIT_VALUE);
        cpu_write_mem(cpu, address, result);
        num_cycles = 10; // NOLINT
        break;
      }
    case 0x36: // NOLINT
      {        // MVI M, D8
        uint16_t address = readRegisterPair(cpu, HL);
        uint8_t value = getImmediate8BitValue(cpu);

        cpu_write_mem(cpu, address, value);

        cpu->pc += 1;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0x37: // NOLINT
      {        // STC
        cpu->flags |= FLAG_CY;
        num_cycles = 4; // NOLINT
        break;
      }
    case 0x3a: // NOLINT
      {        // LDA adr
        uint16_t addr = getImmediate16BitValue(cpu);
        cpu->a = cpu_read_mem(cpu, addr);
        cpu->pc += 2;
        num_cycles = 13; // NOLINT
        break;
      }
    case 0x3c: // NOLINT
      {        // INR A
        num_cycles = INR(cpu, &cpu->a);
        break;
      }
    case 0x3d: // NOLINT
      {        // DCR A
        num_cycles = DCR(cpu, &cpu->a);
        break;
      }
    case 0x3e: // NOLINT
      {        // MVI A
        num_cycles = MVI(&cpu->a, getImmediate8BitValue(cpu));
        cpu->pc += 1;
        break;
      }
    case 0x40: // NOLINT
      {        // MOV B,B
        num_cycles = MOV(&cpu->b, &cpu->b);
        break;
      }
    case 0x41: // NOLINT
      {        // MOV B,C
        num_cycles = MOV(&cpu->b, &cpu->c);
        break;
      }
    case 0x42: // NOLINT
      {        // MOV B,D
        num_cycles = MOV(&cpu->b, &cpu->d);
        break;
      }
    case 0x43: // NOLINT
      {        // MOV B,E
        num_cycles = MOV(&cpu->b, &cpu->e);
        break;
      }
    case 0x44: // NOLINT
      {        // MOV B,H
        num_cycles = MOV(&cpu->b, &cpu->h);
        break;
      }
    case 0x46: // NOLINT
      {        // MOV B,M
        num_cycles = MOV_FROM_MEM(cpu, &cpu->b);
        break;
      }
    case 0x47: // NOLINT
      {        // MOV B,A
        num_cycles = MOV(&cpu->b, &cpu->a);
        break;
      }
    case 0x48: // NOLINT
      {        // MOV C,B
        num_cycles = MOV(&cpu->c, &cpu->b);
        break;
      }
    case 0x4e: // NOLINT
      {        // MOV C,M
        num_cycles = MOV_FROM_MEM(cpu, &cpu->c);
        break;
      }
    case 0x4f: // NOLINT
      {        // MOV C,A
        num_cycles = MOV(&cpu->c, &cpu->a);
        break;
      }
    case 0x56: // NOLINT
      {        // MOV D,M
        num_cycles = MOV_FROM_MEM(cpu, &cpu->d);
        break;
      }
    case 0x57: // NOLINT
      {        // MOV D,A
        num_cycles = MOV(&cpu->d, &cpu->a);
        break;
      }
    case 0x5e: // NOLINT
      {        // MOV E,M
        num_cycles = MOV_FROM_MEM(cpu, &cpu->e);
        break;
      }
    case 0x5f: // NOLINT
      {        // MOV E,A
        num_cycles = MOV(&cpu->e, &cpu->a);
        break;
      }
    case 0x61: // NOLINT
      {        // MOV H,C
        num_cycles = MOV(&cpu->h, &cpu->c);
        break;
      }
    case 0x64: // NOLINT
      {        // MOV H,H
        num_cycles = MOV(&cpu->h, &cpu->h);
        break;
      }
    case 0x65: // NOLINT
      {        // MOV H,L
        num_cycles = MOV(&cpu->h, &cpu->l);
        break;
      }
    case 0x66: // NOLINT
      {        // MOV H,M
        num_cycles = MOV_FROM_MEM(cpu, &cpu->h);
        break;
      }
    case 0x67: // NOLINT
      {        // MOV H,A
        num_cycles = MOV(&cpu->h, &cpu->a);
        break;
      }
    case 0x68: // NOLINT
      {        // MOV L,B
        num_cycles = MOV(&cpu->l, &cpu->b);
        break;
      }
    case 0x69: // NOLINT
      {        // MOV L,C
        num_cycles = MOV(&cpu->l, &cpu->c);
        break;
      }
    case 0x6f: // NOLINT
      {        // MOV L,A
        num_cycles = MOV(&cpu->l, &cpu->a);
        break;
      }
    case 0x70: // NOLINT
      {        // MOV M,B
        num_cycles = MOV_TO_MEM(cpu, &cpu->b);
        break;
      }
    case 0x71: // NOLINT
      {        // MOV M,C
        num_cycles = MOV_TO_MEM(cpu, &cpu->c);
        break;
      }
    case 0x72: // NOLINT
      {        // MOV M,D
        num_cycles = MOV_TO_MEM(cpu, &cpu->d);
        break;
      }
    case 0x73: // NOLINT
      {        // MOV M,E
        num_cycles = MOV_TO_MEM(cpu, &cpu->e);
        break;
      }
    case 0x77: // NOLINT
      {        // MOV M,A
        num_cycles = MOV_TO_MEM(cpu, &cpu->a);
        break;
      }
    case 0x78: // NOLINT
      {        // MOV A,B
        num_cycles = MOV(&cpu->a, &cpu->b);
        break;
      }
    case 0x79: // NOLINT
      {        // MOV A,C
        num_cycles = MOV(&cpu->a, &cpu->c);
        break;
      }
    case 0x7a: // NOLINT
      {        // MOV A,D
        num_cycles = MOV(&cpu->a, &cpu->d);
        break;
      }
    case 0x7b: // NOLINT
      {        // MOV A,E
        num_cycles = MOV(&cpu->a, &cpu->e);
        break;
      }
    case 0x7c: // NOLINT
      {        // MOV A,H
        num_cycles = MOV(&cpu->a, &cpu->h);
        break;
      }
    case 0x7d: // NOLINT
      {        // MOV A,L
        num_cycles = MOV(&cpu->a, &cpu->l);
        break;
      }
    case 0x7e: // NOLINT
      {        // MOV A,M
        num_cycles = MOV_FROM_MEM(cpu, &cpu->a);
        break;
      }
    case 0x80: // NOLINT
      {        // ADD B
        num_cycles = add_reg_accum(cpu, cpu->b);
        break;
      }
    case 0x81: // NOLINT
      {        // ADD C
        num_cycles = add_reg_accum(cpu, cpu->c);
        break;
      }
    case 0x82: // NOLINT
      {        // ADD D
        num_cycles = add_reg_accum(cpu, cpu->d);
        break;
      }
    case 0x83: // NOLINT
      {        // ADD E
        num_cycles = add_reg_accum(cpu, cpu->e);
        break;
      }
    case 0x85: // NOLINT
      {        // ADD L
        num_cycles = add_reg_accum(cpu, cpu->l);
        break;
      }
    case 0x86: // NOLINT
      {        // ADD M
        num_cycles
            = add_reg_accum(cpu, cpu_read_mem(cpu, readRegisterPair(cpu, HL)))
              + 3;
        break;
      }
    case 0x8a: // NOLINT
      {        // ADC D
        num_cycles = ADC(cpu, &cpu->d);
        break;
      }
    case 0x97: // NOLINT
      {        // SUB A
        num_cycles = SUB(cpu, cpu->a);
        break;
      }
    case 0xa0: // NOLINT
      {        // ANA B
        num_cycles = ANA(cpu, cpu->b);
        break;
      }
    case 0xa1: // NOLINT
      {        // ANA C
        num_cycles = ANA(cpu, cpu->c);
        break;
      }
    case 0xa6: // NOLINT
      {        // ANA M
        num_cycles = ANA(cpu, cpu_read_mem(cpu, readRegisterPair(cpu, HL)))
                     + 3; // 7 cycles
        break;
      }
    case 0xa7: // NOLINT
      {        // ANA A
        num_cycles = ANA(cpu, cpu->a);
        break;
      }
    case 0xa8: // NOLINT
      {        // XRA B
        num_cycles = XRA(cpu, &cpu->b);
        break;
      }
    case 0xaf: // NOLINT
      {        // XRA A
        num_cycles = XRA(cpu, &cpu->a);
        break;
      }
    case 0xb0: // NOLINT
      {        // ORA B
        num_cycles = ORA(cpu, cpu->b);
        break;
      }
    case 0xb4: // NOLINT
      {        // ORA H
        num_cycles = ORA(cpu, cpu->h);
        break;
      }
    case 0xb6: // NOLINT
      {        // ORA M
        num_cycles = ORA(cpu, cpu_read_mem(cpu, readRegisterPair(cpu, HL)))
                     + 3; // 7 cycles
        break;
      }
    case 0xb8: // NOLINT
      {        // CMP B
        num_cycles = CMP(cpu, cpu->b);
        break;
      }
    case 0xbc: // NOLINT
      {        // CMP H
        num_cycles = CMP(cpu, cpu->h);
        break;
      }
    case 0xbe: // NOLINT
      {        // CMP M
        num_cycles = CMP(cpu, cpu_read_mem(cpu, readRegisterPair(cpu, HL)))
                     + 3; // 7 cyles
        break;
      }
    case 0xc0:                          // NOLINT
      {                                 // RNZ
        if ((cpu->flags & FLAG_Z) == 0) // if Z reset, RET
          {
            return RET(cpu) + 1; // 11 cycles
          }
        num_cycles = 5; // NOLINT
        break;
      }
    case 0xc1: // NOLINT
      {        // POP B
        num_cycles = POP(cpu, BC);
        break;
      }
    case 0xc2: // NOLINT
      {        // JNZ
        if ((cpu->flags & FLAG_Z) == 0)
          {
            return JMP(cpu);
          }
        cpu->pc += 2;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xc3: // NOLINT
      {        // JMP
        return JMP(cpu);
      }
    case 0xc4: // NOLINT
      {        // CNZ
        if ((cpu->flags & FLAG_Z) == 0)
          {
            return CALL(cpu, getImmediate16BitValue(cpu));
          }
        cpu->pc += 2;
        num_cycles = 11; // NOLINT
        break;
      }
    case 0xc5: // NOLINT
      {        // PUSH B
        num_cycles = PUSH(cpu, BC);
        break;
      }
    case 0xc6: // NOLINT
      {        // ADI
        uint8_t immediate = getImmediate8BitValue(cpu);
        uint16_t answer = cpu->a + immediate;
        update_zero_flag(cpu, (uint8_t)answer);
        update_sign_flag(cpu, (uint8_t)answer);
        update_parity_flag(cpu, (uint8_t)answer);
        update_carry_flag(cpu, answer > MAX_8_BIT_VALUE);
        update_aux_carry_flag(cpu, cpu->a, immediate);
        cpu->a = (uint8_t)(answer & LOWER_8_BIT_MASK);
        cpu->pc += 1;
        num_cycles = 7; // NOLINT
        break;
      }
    case 0xc8:                               // NOLINT
      {                                      // RZ
        if ((cpu->flags & FLAG_Z) == FLAG_Z) // if Z set, RET
          {
            return RET(cpu) + 1; // 11 cycles
          }
        num_cycles = 5; // NOLINT
        break;
      }
    case 0xc9: // NOLINT
      {        // RET
        return RET(cpu);
      }
    case 0xca: // NOLINT
      {        // JZ
        if (is_zero_flag_set(cpu))
          {
            return JMP(cpu);
          }
        cpu->pc += 2;    // NOLINT
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xcc: // NOLINT
      {        // CZ ADDR
        if ((cpu->flags & FLAG_Z) == FLAG_Z)
          {
            return CALL(cpu, getImmediate16BitValue(cpu));
          }
        cpu->pc += 2;
        num_cycles = 11; // NOLINT
        break;
      }
    case 0xcd: // NOLINT
      {        // CALL ADDR
        return CALL(cpu, getImmediate16BitValue(cpu));
      }
    case 0xd0: // NOLINT
      {        // RNC
        if ((cpu->flags & FLAG_CY) == 0)
          {
            return RET(cpu) + 1; // 11 cycles
          }
        num_cycles = 5;
        break;
      }
    case 0xd1: // NOLINT
      {        // POP D
        num_cycles = POP(cpu, DE);
        break;
      }
    case 0xd2:                                 // NOLINT
      {                                        // JNC ADR
        if ((cpu->flags & FLAG_CY) != FLAG_CY) // if CY not set JUMP
          {
            return JMP(cpu);
          }
        cpu->pc += 2;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xd3: // NOLINT
      {        // OUT d8
        uint8_t port = getImmediate8BitValue(cpu);
        port_out(cpu, port, cpu->a);
        cpu->pc += 1;
        num_cycles = 10;
        break;
      }
    case 0xd4: // NOLINT
      {        // CNC ADDR
        if ((cpu->flags & FLAG_CY) == 0)
          {
            return CALL(cpu, getImmediate16BitValue(cpu));
          }
        cpu->pc += 2;
        num_cycles = 11; // NOLINT
        break;
      }
    case 0xd5: // NOLINT
      {        // PUSH D
        num_cycles = PUSH(cpu, DE);
        break;
      }
    case 0xd6:                                                 // NOLINT
      {                                                        // SUI d8
        num_cycles = SUB(cpu, getImmediate8BitValue(cpu)) + 3; // 7 cycles
        cpu->pc += 1;
        break;
      }
    case 0xd8: // NOLINT
      {        // RC
        if ((cpu->flags & FLAG_CY) == FLAG_CY)
          {
            return RET(cpu) + 1; // 11 cycles
          }
        num_cycles = 5;
        break;
      }
    case 0xda:                                 // NOLINT
      {                                        // JC ADR
        if ((cpu->flags & FLAG_CY) == FLAG_CY) // if CY set JUMP
          {
            return JMP(cpu);
          }
        cpu->pc += 2;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xdb: // NOLINT
      {        // IN D8
        uint8_t port = getImmediate8BitValue(cpu);
        cpu->a = port_in(cpu, port);
        cpu->pc += 1;
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xde: // NOLINT
      {
        num_cycles = SBI(cpu, getImmediate8BitValue(cpu));
        cpu->pc += 1;
        break;
      }
    case 0xe1: // NOLINT
      {        // POP H
        num_cycles = POP(cpu, HL);
        break;
      }
    case 0xe3: // NOLINT
      {        // XTHL

        uint8_t tmp = cpu->h;
        cpu->h = cpu_read_mem(cpu, cpu->sp + 1);
        cpu_write_mem(cpu, cpu->sp + 1, tmp);
        tmp = cpu->l;
        cpu->l = cpu_read_mem(cpu, cpu->sp);
        cpu_write_mem(cpu, cpu->sp, tmp);

        num_cycles = 18; // NOLINT
        break;
      }
    case 0xe5: // NOLINT
      {        // PUSH H
        num_cycles = PUSH(cpu, HL);
        break;
      }
    case 0xe6: // NOLINT
      {        // ANI d8
        uint8_t immediate = getImmediate8BitValue(cpu);
        cpu->a &= immediate;
        update_zero_flag(cpu, cpu->a);
        update_sign_flag(cpu, cpu->a);
        update_parity_flag(cpu, cpu->a);
        update_carry_flag(cpu, false);
        cpu->flags &= ~FLAG_AC;
        cpu->pc += 1;
        num_cycles = 7; // NOLINT
        break;
      }
    case 0xe9: // NOLINT
      {        // PCHL
        cpu->pc = readRegisterPair(cpu, HL);
        return 5; // NOLINT
      }
    case 0xeb: // NOLINT
      {        // XCHG
        // exchange h and d
        uint8_t temp = cpu->h;
        cpu->h = cpu->d;
        cpu->d = temp;

        // exchange l and e
        temp = cpu->l;
        cpu->l = cpu->e;
        cpu->e = temp;
        num_cycles = 5; // NOLINT
        break;
      }
    case 0xf1: // NOLINT
      {        // POP PSW
        num_cycles = POP(cpu, PSW);
        break;
      }
    case 0xf5: // NOLINT
      {        // PUSH PSW
        num_cycles = PUSH(cpu, PSW);
        break;
      }
      break;
    case 0xf6: // NOLINT
      {        // ORI d8
        uint8_t immediate = getImmediate8BitValue(cpu);
        cpu->a |= immediate;
        update_zero_flag(cpu, cpu->a);
        update_sign_flag(cpu, cpu->a);
        update_parity_flag(cpu, cpu->a);
        update_carry_flag(cpu, false);
        cpu->flags &= ~FLAG_AC;
        cpu->pc += 1;
        num_cycles = 7; // NOLINT
        break;
      }
    case 0xfa: // NOLINT
      {        // JM
        if (is_sign_flag_set(cpu))
          {
            return JMP(cpu);
          }
        cpu->pc += 2;    // NOLINT
        num_cycles = 10; // NOLINT
        break;
      }
    case 0xfb: // NOLINT
      {        // EI
        cpu->interrupt_enabled = true;
        num_cycles = 4; // NOLINT
        break;
      }
    case 0xfe: // NOLINT
      {        // CPI
        uint8_t data = getImmediate8BitValue(cpu);
        uint8_t result = cpu->a - data;
        update_zero_flag(cpu, result);
        update_sign_flag(cpu, result);
        update_parity_flag(cpu, result);
        update_carry_flag(cpu, (data > cpu->a));
        update_aux_carry_flag(cpu, cpu->a, (~data + 1));
        cpu->pc += 1;
        num_cycles = 7; // NOLINT
        break;
      }
    default:
      {
        fprintf(stderr, "Error: opcode 0x%02x not found\n", opcode);
        return -1;
      }
    }
  cpu->pc += 1;
  return num_cycles;
}
void
load_sound(const char *soundFilePath, Mix_Chunk **sound)
{
  *sound = Mix_LoadWAV(soundFilePath);
  if (*sound == NULL)
    {
      fprintf(stderr, "Failed to load sound file!");
    }
}
void
play_sound(i8080 *cpu, uint8_t bank)
{
  uint8_t data = cpu->a;
  int sound_to_play = -1;

  if (bank == 1)
    {
      if (data != cpu->last_out_port3)
        {
          if ((data & 0x1) && !(cpu->last_out_port3 & 0x1))
            {
              sound_to_play = 0;
            }
          if ((data & 0x2) && !(cpu->last_out_port3 & 0x2))
            {
              sound_to_play = 1;
            }
          if ((data & 0x4) && !(cpu->last_out_port3 & 0x4))
            {
              sound_to_play = 2;
            }
          if ((data & 0x8) && !(cpu->last_out_port3 & 0x8))
            {
              sound_to_play = 3;
            }
          cpu->last_out_port3 = data;
        }
    }
  else if (bank == 2)
    {
      if (data != cpu->last_out_port5)
        {
          if ((data & 0x1) && !(cpu->last_out_port5 & 0x1))
            {
              sound_to_play = 4;
            }
          if ((data & 0x2) && !(cpu->last_out_port5 & 0x2))
            {
              sound_to_play = 5;
            }
          if ((data & 0x4) && !(cpu->last_out_port5 & 0x4))
            {
              sound_to_play = 6;
            }
          if ((data & 0x8) && !(cpu->last_out_port5 & 0x8))
            {
              sound_to_play = 7;
            }
          if ((data & 0x10) && !(cpu->last_out_port5 & 0x10))
            {
              sound_to_play = 8;
            }
          cpu->last_out_port5 = data;
        }
    }
  if (sound_to_play != -1 && cpu->sounds[sound_to_play] != NULL)
    {
      Mix_PlayChannel(-1, cpu->sounds[sound_to_play], 0);
    }
}
void
cpu_init(i8080 *cpu)
{
  cpu->a = 0;
  cpu->b = 0;
  cpu->c = 0;
  cpu->d = 0;
  cpu->e = 0;
  cpu->h = 0;
  cpu->l = 0;

  cpu->flags = 0;
  cpu->pc = 0;
  cpu->sp = 0;
  cpu->interrupt_enabled = false;
  cpu->halted = false;

  cpu->port1 = 0;
  cpu->port2 = 0;

  cpu->shift_msb = 0;
  cpu->shift_lsb = 0;
  cpu->shift_offset = 0;

  cpu->last_out_port3 = 0;
  cpu->last_out_port5 = 0;
  load_sound("sounds/8.wav", &cpu->sounds[0]);
  load_sound("sounds/1.wav", &cpu->sounds[1]);
  load_sound("sounds/2.wav", &cpu->sounds[2]);
  load_sound("sounds/3.wav", &cpu->sounds[3]);
  load_sound("sounds/4.wav", &cpu->sounds[4]);
  load_sound("sounds/5.wav", &cpu->sounds[5]);
  load_sound("sounds/6.wav", &cpu->sounds[6]);
  load_sound("sounds/7.wav", &cpu->sounds[7]);
  load_sound("sounds/0.wav", &cpu->sounds[8]);
}

uint8_t
cpu_read_mem(i8080 *cpu, uint16_t address)
{
  return cpu->memory[address];
}

void
cpu_write_mem(i8080 *cpu, uint16_t address, uint8_t data)
{
  cpu->memory[address] = data;
}

bool
cpu_load_file(i8080 *cpu, const char *file_path, uint16_t address)
{
  FILE *file = fopen(file_path, "rb");

  if (file == NULL)
    {
      fprintf(stderr, "Error: Unable to open file %s\n", file_path);
      return false;
    }

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);

  if (file_size < 0)
    {
      perror("Error: unable to obtain file size");
      return false;
    }

  fseek(file, 0, SEEK_SET);

  if (address + file_size > MEM_SIZE)
    {
      fprintf(stderr, "Error: File size exceeds available memory\n");
      fclose(file);
      return false;
    }

  size_t bytes_read = fread(&cpu->memory[address], 1, file_size, file);
  fclose(file);

  if (bytes_read != file_size)
    {
      fprintf(stderr, "Error: Unable to read the entire file into memory\n");
      return false;
    }

  return true;
}

// Input/Output

static uint8_t
port_in(i8080 *cpu, uint8_t port)
{
  uint8_t value = 0xFF;

  switch (port)
    {
    case 0:
      break;
    case 1:
      value = cpu->port1;
      break;
    case 2:
      value = cpu->port2;
      break;
    case 3:
      {
        const uint16_t shift = (cpu->shift_msb << 8) | cpu->shift_lsb;
        value = (shift >> (8 - cpu->shift_offset)) & 0xFF;
        break;
      }
    default:
      fprintf(stderr, "Error: unknown IN port %02x\n", port);
      break;
    }
  return value;
}

static void
port_out(i8080 *cpu, uint8_t port, uint8_t value)
{
  switch (port)
    {
    case 2:
      cpu->shift_offset = value & 7;
      break;
    case 3:
      // play sound 1 from sound bank
      play_sound(cpu, 1);
      break;
    case 4:
      cpu->shift_lsb = cpu->shift_msb;
      cpu->shift_msb = value;
      break;
    case 5:
      play_sound(cpu, 2);
      break;
    case 6:
      break;
    default:
      fprintf(stderr, "Error: Unknown OUT port %02x\n", port);
      break;
    }
}

// FLAGS

void
update_aux_carry_flag(i8080 *cpu, uint8_t a, uint8_t b)
{
  // Masks highest 4 bits which preserves the nibbles(last 4 bits of a and b)
  // then adds nibbles to test for AC
  uint16_t result = (a & 0x0F) + (b & 0x0F); // NOLINT
  if (result & 0x10)                         // NOLINT
    { // Check if carry from bit 3 to bit 4 exists
      cpu->flags |= FLAG_AC;
    }
  else
    {
      cpu->flags &= ~FLAG_AC;
    }
}

void
update_zero_flag(i8080 *cpu, uint8_t result)
{
  if (result == 0)
    {
      cpu->flags |= FLAG_Z;
    }
  else
    {
      cpu->flags &= ~FLAG_Z;
    }
}

void
update_carry_flag(i8080 *cpu, bool carry_occurred)
{
  if (carry_occurred)
    {
      cpu->flags |= FLAG_CY;
    }
  else
    {
      cpu->flags &= ~FLAG_CY;
    }
}

int
count_set_bits(uint8_t value)
{
  int count = 0;
  for (int i = 0; i < 8; i++) // NOLINT
    {
      if (value & (1 << i))
        {
          count++;
        }
    }
  return count;
}

void
update_parity_flag(i8080 *cpu, uint8_t result)
{
  int set_bits = count_set_bits(result);
  if (set_bits % 2 == 0)
    {
      cpu->flags |= FLAG_P;
    }
  else
    {
      cpu->flags &= ~FLAG_P;
    }
}

bool
is_sign_flag_set(i8080 *cpu)
{
  return (cpu->flags & FLAG_S) != 0;
}

bool
is_zero_flag_set(i8080 *cpu)
{
  return (cpu->flags & FLAG_Z) != 0;
}

void
update_sign_flag(i8080 *cpu, uint8_t result)
{
  if (result & 0x80) // NOLINT
    {
      cpu->flags |= FLAG_S;
    }
  else
    {
      cpu->flags &= ~FLAG_S;
    }
}

// Interrupts

// UPDATE GRAPHICS

void
update_graphics(i8080 *cpu, SDL_Surface *buffer, SDL_Surface *surface)
{

  uint32_t *screen_buff = buffer->pixels;

  // Graphics data is rotated 90 degrees in memory counter-clockwise.  Reading
  // byte by byte starting at 0x2400 we need to fill in the screen left to
  // right, bottom to top.
  int vram = 0x2400; // NOLINT
  // Start at the left edge
  for (int column = 0; column < SCREEN_WIDTH; column++)
    {
      // Start at bottom of screen, decrement by 8 since each bit is a pixel.
      for (int row = SCREEN_HEIGHT; row > 0; row -= 8) // NOLINT
        {
          // Set each pixel based on bit value.
          for (int pixel = 0; pixel < 8; pixel++) // NOLINT
            {
              // Calculate surface index.
              int surf_index = (SCREEN_WIDTH * (row - pixel)) + column
                               - (SCREEN_WIDTH - 1);

              uint8_t cur_byte = cpu_read_mem(cpu, vram);

              // Set pixel to on by changing color to white.
              if ((cur_byte >> pixel) & 1)
                {
                  screen_buff[surf_index] = 0xFFFFFF; // NOLINT
                }
              else
                {
                  // Set pixel to off by changing color to black.
                  screen_buff[surf_index] = 0x000000; // NOLINT
                }
            }
          vram++; // Increment to next byte in VRAM
        }
    }

  // Convert buffer to scaled screen surface.
  SDL_Surface *scaled_surface = NULL;
  scaled_surface = SDL_ConvertSurface(buffer, surface->format, 0);

  // Copy scaled surface to screen.
  SDL_BlitScaled(scaled_surface, NULL, surface, NULL);
}

// DEBUGGING FUNCTIONS

void
print_instruction(uint8_t opcode)
{
  printf("INSTRUCTION: 0x%02x\n", opcode);
}

void
print_state(i8080 *cpu)
{
  printf("REGISTERS a: 0x%02x b: 0x%02x c: 0x%02x d: 0x%02x e: 0x%02x h: "
         "0x%02x l: 0x%02x sp: 0x%04x pc: 0x%04x ",
         cpu->a, cpu->b, cpu->c, cpu->d, cpu->e, cpu->h, cpu->l, cpu->sp,
         cpu->pc);
}

void
print_flags(uint8_t flags)
{
  printf("FLAGS z: %d s: %d p: %d cy: %d ac %d", (flags & FLAG_Z) == FLAG_Z,
         (flags & FLAG_S) == FLAG_S, (flags & FLAG_P) == FLAG_P,
         (flags & FLAG_CY) == FLAG_CY, (flags & FLAG_AC) == FLAG_AC);
}
// Sound Load
// INTERRUPTS
int
handle_interrupt(i8080 *cpu, uint8_t rst_instruction)
{
  if (rst_instruction > RST_RANGE)
    {
      fprintf(stderr, "Invalid restart instruction %u", rst_instruction);
      return -1;
    }

  if (cpu->interrupt_enabled == false)
    {
      return 0;
    }

  // get address for interrupt subroutine
  uint16_t subroutine_address = BYTE * rst_instruction;

  // push program counter to stack
  cpu_write_mem(cpu, cpu->sp - 1,
                (uint8_t)((cpu->pc & UPPER_8_BIT_MASK) >> BYTE));
  cpu_write_mem(cpu, cpu->sp - 2, (uint8_t)(cpu->pc & LOWER_8_BIT_MASK));
  cpu->sp -= 2;

  // set program counter to start of the interrupt subroutine
  cpu->pc = subroutine_address;

  // disable interrupts
  cpu->interrupt_enabled = false;

  return 0;
}
