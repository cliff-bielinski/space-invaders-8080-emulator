#include "emulator.h"
#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Open any necessary files for test suite here */
int
init_opcodes_suite(void)
{
  return 0;
}

/* Close anything that was opened here for test suite */
int
clean_suite(void)
{
  return 0;
}

/* Add tests here */

void
test_opcode_0x00(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  int code_found = execute_instruction(&cpu, 0x00);
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 1);
}

void
test_opcode_0x01(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu_write_mem(&cpu, 0x0001, 0xFF); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0x12); // NOLINT

  int code_found = execute_instruction(&cpu, 0x01); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 3);
  CU_ASSERT(cpu.c == 0xFF);
  CU_ASSERT(cpu.b == 0x12);

  // clean up
  cpu_write_mem(&cpu, 0x0001, 0x00);
  cpu_write_mem(&cpu, 0x0002, 0x00);
}

void
test_opcode_0x06(void) // NOLINT
{                      // MVI B, mem

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu_write_mem(&cpu, 0x0001, 0xCC); // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0x06); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(2 == cpu.pc);
  CU_ASSERT(0xCC == cpu.b);

  // cleanup
  cpu_write_mem(&cpu, 0x0001, 0x00);
}

void
test_opcode_0x09(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu.b = 0x12; // NOLINT
  cpu.c = 0x34; // NOLINT
  cpu.h = 0xAB; // NOLINT
  cpu.l = 0xCD; // NOLINT

  uint16_t initial_hl = (cpu.h << 8) | cpu.l;               // NOLINT
  uint16_t initial_bc = (cpu.b << 8) | cpu.c;               // NOLINT
  int code_found = execute_instruction(&cpu, 0x09);         // NOLINT
  uint16_t result_hl = (cpu.h << 8) | cpu.l;                // NOLINT
  bool carry_occurred = (initial_hl + initial_bc) > 0xFFFF; // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 1);

  CU_ASSERT(result_hl == initial_hl + initial_bc);
  CU_ASSERT((cpu.flags & FLAG_CY) == (carry_occurred ? FLAG_CY : 0));
  cpu.b = 0;
  cpu.c = 0;
  cpu.h = 0;
  cpu.l = 0;
}

void
test_opcode_0x0d(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu.c = 0x02;

  // decrease C from 2 to 1
  int code_found = execute_instruction(&cpu, 0x0d); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu.c == 0x01);
  CU_ASSERT((cpu.flags & FLAG_P) == 0);
  CU_ASSERT((cpu.flags & FLAG_S) == 0);
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);
  CU_ASSERT((cpu.flags & FLAG_AC) == FLAG_AC);

  // decrease C from 1 to 0
  code_found = execute_instruction(&cpu, 0x0d); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 2);
  CU_ASSERT(cpu.c == 0x00);
  CU_ASSERT((cpu.flags & FLAG_P) == FLAG_P);
  CU_ASSERT((cpu.flags & FLAG_S) == 0);
  CU_ASSERT((cpu.flags & FLAG_Z) == FLAG_Z);
  CU_ASSERT((cpu.flags & FLAG_AC) == FLAG_AC);

  // decrease C from 0 to 255
  code_found = execute_instruction(&cpu, 0x0d); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 3);
  CU_ASSERT(cpu.c == 0xFF);
  CU_ASSERT((cpu.flags & FLAG_P) == FLAG_P);
  CU_ASSERT((cpu.flags & FLAG_S) == FLAG_S);
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);
  CU_ASSERT((cpu.flags & FLAG_AC) == 0);
}

void
test_opcode_0x0f(void) // NOLINT
{                      // RRC

  /* no carry test */
  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu.a = 0xCC; // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0x0f); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0x66 == cpu.a); // NOLINT
  CU_ASSERT(FLAG_CY != (cpu.flags & FLAG_CY));

  /* carry test */
  // setup
  cpu.a = 0x3D; // NOLINT

  // execute
  code_found = execute_instruction(&cpu, 0x0f); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(2 == cpu.pc);
  CU_ASSERT(0x9E == cpu.a); // NOLINT
  CU_ASSERT(FLAG_CY == (cpu.flags & FLAG_CY));
}

void
test_opcode_0x11(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu_write_mem(&cpu, 0x0001, 0x34); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0x12); // NOLINT

  int code_found = execute_instruction(&cpu, 0x11); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 3);

  CU_ASSERT(cpu.e == 0x34);
  CU_ASSERT(cpu.d == 0x12);
  cpu.d = 0;
  cpu.e = 0;
  cpu_write_mem(&cpu, 0x0001, 0x00);
  cpu_write_mem(&cpu, 0x0002, 0x00);
}

void
test_opcode_0x13(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  int code_found = execute_instruction(&cpu, 0x13); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu.e == 1);
  CU_ASSERT(cpu.d == 0);

  cpu.e = 0xFF;                                 // NOLINT
  code_found = execute_instruction(&cpu, 0x13); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 2);
  CU_ASSERT(cpu.e == 0);
  CU_ASSERT(cpu.d == 1);
}

void
test_opcode_0x05(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu.b = 0xF;                                      // NOLINT
  int code_found = execute_instruction(&cpu, 0x05); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT_EQUAL(cpu.b, 14);
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);
  CU_ASSERT((cpu.flags & FLAG_S) == 0);
  CU_ASSERT((cpu.flags & FLAG_P) == 0);
  CU_ASSERT((cpu.flags & FLAG_AC) == FLAG_AC);
}

void
test_opcode_0x0e(void)
{
  // MVI C, D8
  i8080 cpu;
  cpu_init(&cpu);

  cpu.pc = 0x4341;                       // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 1, 0x19); // NOLINT

  int code_found = execute_instruction(&cpu, 0x0e); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT_EQUAL(cpu.c, 0x19); // NOLINT

  cpu_write_mem(&cpu, 0x4342, 0x00); // NOLINT
}

void
test_opcode_0x19(void) // NOLINT
{
  // DAD D
  i8080 cpu;
  cpu_init(&cpu);

  cpu.d = 0xFF; // NOLINT
  cpu.e = 0x4E; // NOLINT
  cpu.h = 0x3F; // NOLINT
  cpu.l = 0xC8; // NOLINT

  // Carry set test
  int code_found = execute_instruction(&cpu, 0x19); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.h == 0x3F); // NOLINT
  CU_ASSERT(cpu.l == 0x16); // NOLINT
  CU_ASSERT((cpu.flags & FLAG_CY) == FLAG_CY);

  // Carry not set test
  cpu.d = 0x23; // NOLINT
  cpu.e = 0x00; // NOLINT
  cpu.h = 0x3F; // NOLINT
  cpu.l = 0xC8; // NOLINT

  code_found = execute_instruction(&cpu, 0x19); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.h == 0x62); // NOLINT
  CU_ASSERT(cpu.l == 0xC8); // NOLINT
  CU_ASSERT((cpu.flags & FLAG_CY) == 0);
}

void
test_opcode_0x21(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu_write_mem(&cpu, 0x0001, 0x78); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0x56); // NOLINT

  int code_found = execute_instruction(&cpu, 0x21); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 3);

  CU_ASSERT(cpu.l == 0x78);
  CU_ASSERT(cpu.h == 0x56);
  cpu.h = 0;
  cpu.l = 0;
  cpu_write_mem(&cpu, 0x0001, 0x00);
  cpu_write_mem(&cpu, 0x0002, 0x00);
}

void
test_opcode_0x26(void)
{
  // MVI H, D8
  i8080 cpu;
  cpu_init(&cpu);

  cpu.pc = 0x4563;                       // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 1, 0x65); // NOLINT

  int code_found = execute_instruction(&cpu, 0x26); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT_EQUAL(cpu.h, 0x65); // NOLINT

  cpu_write_mem(&cpu, 0x4564, 0x00); // NOLINT
}

void
test_opcode_0x31(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu_write_mem(&cpu, 0x0001, 0xAB); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0xCD); // NOLINT

  int code_found = execute_instruction(&cpu, 0x31); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 3);

  CU_ASSERT(cpu.sp == 0xCDAB);
  cpu.sp = 0;

  cpu_write_mem(&cpu, 0x0001, 0x00);
  cpu_write_mem(&cpu, 0x0002, 0x00);
}

void
test_opcode_0x36(void)
{
  // MOV M, D8
  i8080 cpu;
  cpu_init(&cpu);

  cpu.pc = 0x4352;                       // NOLINT
  cpu.h = 0x23;                          // NOLINT
  cpu.l = 0x12;                          // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 1, 0xA1); // NOLINT

  int code_found = execute_instruction(&cpu, 0x36); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT_EQUAL(cpu_read_mem(&cpu, 0x2312), 0xA1); // NOLINT

  cpu_write_mem(&cpu, 0x4353, 0x00); // NOLINT
}

void
test_opcode_0x3e(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu_write_mem(&cpu, 0x0001, 0xEF); // NOLINT

  int code_found = execute_instruction(&cpu, 0x3e); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 2);

  CU_ASSERT(cpu.a == 0xEF);
  cpu.a = 0;

  cpu_write_mem(&cpu, 0x0001, 0x00);
}

void
test_opcode_0x6f(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu.a = 0xBD; // NOLINT

  int code_found = execute_instruction(&cpu, 0x6f); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 1);
  CU_ASSERT(cpu.l == 0xBD);

  cpu.a = 0;
  cpu.l = 0;
}

void
test_opcode_0xc1(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu.sp = 0x1000;                       // NOLINT
  cpu_write_mem(&cpu, cpu.sp, 0x78);     // NOLINT
  cpu_write_mem(&cpu, cpu.sp + 1, 0x56); // NOLINT

  int code_found = execute_instruction(&cpu, 0xc1); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 1);
  CU_ASSERT(cpu.sp == 0x1002);
  CU_ASSERT(cpu.c == 0x78);
  CU_ASSERT(cpu.b == 0x56);

  cpu.sp = 0;
  cpu.b = 0;
  cpu.c = 0;
  cpu_write_mem(&cpu, 0x1000, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0x1001, 0x00); // NOLINT
}

void
test_opcode_0xc6(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu.a = 0x50;                      // NOLINT
  cpu_write_mem(&cpu, 0x0001, 0x28); // NOLINT

  int code_found = execute_instruction(&cpu, 0xc6); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 2);
  CU_ASSERT(cpu.a == 0x78);

  // Flags
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);
  CU_ASSERT((cpu.flags & FLAG_S) == 0);
  CU_ASSERT((cpu.flags & FLAG_P) != 0);
  CU_ASSERT((cpu.flags & FLAG_CY) == 0);
  CU_ASSERT((cpu.flags & FLAG_AC) == 0);

  cpu.a = 0;
  cpu_write_mem(&cpu, 0x0001, 0x00);
}

void
test_opcode_0xca(void)
{ // JZ
  i8080 cpu;
  cpu_init(&cpu);

  // case where zero flag not set
  cpu.pc = 0x4341;                                  // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 1, 0x28);            // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 2, 0xb7);            // NOLINT
  update_zero_flag(&cpu, 1);                        // had a remainder
  int code_found = execute_instruction(&cpu, 0xca); // NOLINT
  CU_ASSERT(code_found >= 0);
  CU_ASSERT(cpu.pc == 0x4344); // NOLINT

  // case where zero flag set
  cpu.pc = 0x5441;                              // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 1, 0x3a);        // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 2, 0xa9);        // NOLINT
  update_zero_flag(&cpu, 0);                    // no remainder
  code_found = execute_instruction(&cpu, 0xca); // NOLINT
  CU_ASSERT(code_found >= 0);
  CU_ASSERT(cpu.pc == 0xa93a); // NOLINT

  // cleanup
  cpu_write_mem(&cpu, 0x4342, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0x4343, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0x5442, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0x5443, 0x00); // NOLINT
}

void
test_opcode_0xd3(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu.a = 0x50; // NOLINT
  cpu_write_mem(&cpu, 0x0001, 0x01);

  int code_found = execute_instruction(&cpu, 0xd3); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 2);

  cpu.a = 0;
  cpu_write_mem(&cpu, 0x0001, 0x00);
}

void
test_opcode_0xdb(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu.a = 0x50; // NOLINT
  cpu_write_mem(&cpu, 0x0001, 0x01);

  int code_found = execute_instruction(&cpu, 0xdb); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 2);
  // TODO: check contents of A register to see if byte received from port 1

  cpu_write_mem(&cpu, 0x0001, 0x00);
}

void
test_opcode_0x7c(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu.h = 0xBD; // NOLINT

  int code_found = execute_instruction(&cpu, 0x7c); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 1);
  CU_ASSERT(cpu.a == 0xBD);

  cpu.a = 0;
  cpu.h = 0;
}

void
test_opcode_0xe6(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu.a = 0x5F;                      // NOLINT
  cpu_write_mem(&cpu, 0x0001, 0x3C); // NOLINT

  int code_found = execute_instruction(&cpu, 0xe6); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == initial_pc + 2);
  CU_ASSERT(cpu.a == 0x1C);

  // Flag Asserts
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);
  CU_ASSERT((cpu.flags & FLAG_S) == 0);
  CU_ASSERT((cpu.flags & FLAG_P) == 0);
  CU_ASSERT((cpu.flags & FLAG_CY) == 0);
  CU_ASSERT((cpu.flags & FLAG_AC) == 0);

  cpu.a = 0;
  cpu_write_mem(&cpu, 0x0001, 0x00);
}

void
test_opcode_0xfb(void)
{
  i8080 cpu;
  cpu_init(&cpu);
  uint16_t initial_pc = cpu.pc;
  cpu.interrupt_enabled = false;

  int code_found = execute_instruction(&cpu, 0xfb); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.interrupt_enabled == true);
  CU_ASSERT(cpu.pc == (initial_pc + 1));
}

void
test_opcode_0x5e(void)
{
  // MOV E, M
  i8080 cpu;
  cpu_init(&cpu);

  cpu.h = 0x12;                      // NOLINT
  cpu.l = 0x31;                      // NOLINT
  cpu_write_mem(&cpu, 0x1231, 0x45); // NOLINT

  int code_found = execute_instruction(&cpu, 0x5e); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT_EQUAL(cpu.e, 0x45); // NOLINT

  cpu_write_mem(&cpu, 0x1231, 0x00); // NOLINT
}

void
test_opcode_0x7a(void) // NOLINT
{
  // MOV A,D
  i8080 cpu;
  cpu_init(&cpu);

  cpu.d = 100; // NOLINT

  int code_found = execute_instruction(&cpu, 0x7a); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.a == 100);
}

void
test_opcode_0xa7(void) // NOLINT
{
  // ANA A
  i8080 cpu;
  cpu_init(&cpu);

  cpu.a = 85; // NOLINT
  // S and CY flag set.
  cpu.flags = 0x88; // NOLINT

  int code_found = execute_instruction(&cpu, 0xa7); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.a == 85);
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);
  CU_ASSERT((cpu.flags & FLAG_S) == 0);
  CU_ASSERT((cpu.flags & FLAG_P) == FLAG_P);
  CU_ASSERT((cpu.flags & FLAG_AC) == 0);
  CU_ASSERT((cpu.flags & FLAG_CY) == 0);
}

void
test_opcode_0xe1(void) // NOLINT
{
  // POP H
  i8080 cpu;
  cpu_init(&cpu);

  cpu.sp = 0xDADE;                       // NOLINT
  cpu_write_mem(&cpu, cpu.sp, 0xBC);     // NOLINT
  cpu_write_mem(&cpu, cpu.sp + 1, 0xFE); // NOLINT

  int code_found = execute_instruction(&cpu, 0xe1); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.l == 0xBC);    // NOLINT
  CU_ASSERT(cpu.h == 0xFE);    // NOLINT
  CU_ASSERT(cpu.sp == 0xDAE0); // NOLINT

  cpu_write_mem(&cpu, 0xDADE, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0xDADF, 0x00); // NOLINT
}

void
test_opcode_0xf1(void) // NOLINT
{
  // POP PSW
  i8080 cpu;
  cpu_init(&cpu);

  cpu.sp = 0xCBDE;                       // NOLINT
  cpu_write_mem(&cpu, cpu.sp, 0xA8);     // NOLINT
  cpu_write_mem(&cpu, cpu.sp + 1, 0xDE); // NOLINT
  cpu.flags = 0x50;                      // NOLINT
  cpu.a = 0x19;                          // NOLINT

  int code_found = execute_instruction(&cpu, 0xf1); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT_EQUAL(cpu.flags, 0xA8); // NOLINT
  CU_ASSERT_EQUAL(cpu.a, 0xDE);     // NOLINT

  cpu_write_mem(&cpu, 0xCBDE, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0xCBDF, 0x00); // NOLINT
}

void
test_opcode_0xcd(void) // NOLINT
{
  // Call addr
  i8080 cpu;
  cpu_init(&cpu);

  cpu.pc = 0xABCD;                       // NOLINT
  cpu.sp = 0xDCBA;                       // NOLINT
  uint16_t temp = cpu.sp;                // Holds original sp addr
  cpu_write_mem(&cpu, cpu.pc + 1, 0xFE); // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 2, 0xEF); // NOLINT

  int code_found = execute_instruction(&cpu, 0xcd); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT_EQUAL(cpu_read_mem(&cpu, temp - 1), 0xAB); // NOLINT
  CU_ASSERT_EQUAL(cpu_read_mem(&cpu, temp - 2), 0xD0); // NOLINT
  CU_ASSERT_EQUAL(cpu.sp, 0xDCBA - 2);                 // NOLINT
  CU_ASSERT_EQUAL(cpu.pc, 0xEFFE);                     // NOLINT

  cpu_write_mem(&cpu, 0xABCE, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0xABCF, 0x00); // NOLINT
}

void
test_opcode_0xc3(void) // NOLINT
{
  // JMP
  i8080 cpu;
  cpu_init(&cpu);

  cpu.pc = 0xCDEB;                       // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 1, 0x1E); // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 2, 0x9c); // NOLINT

  int code_found = execute_instruction(&cpu, 0xc3); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT_EQUAL(cpu.pc, 0x9C1E); // NOLINT

  cpu_write_mem(&cpu, 0xCDEC, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0xCDED, 0x00); // NOLINT
}

void
test_opcode_0xc8(void)
{ // RZ
  i8080 cpu;
  cpu_init(&cpu);

  // case where zero flag not set
  cpu.pc = 0x4341;                                  // NOLINT
  update_zero_flag(&cpu, 1);                        // had a remainder
  int code_found = execute_instruction(&cpu, 0xc8); // NOLINT
  CU_ASSERT(code_found >= 0);
  CU_ASSERT(cpu.pc == 0x4342); // NOLINT

  // case where zero flag set
  cpu_write_mem(&cpu, 0x0001, 0xAA); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0xBB); // NOLINT
  cpu.sp = 0x0001;
  update_zero_flag(&cpu, 0); // no remainder

  code_found = execute_instruction(&cpu, 0xc8); // NOLINT

  CU_ASSERT(code_found >= 0);
  CU_ASSERT(cpu.pc == 0xBBAA); // NOLINT
  CU_ASSERT(cpu.sp == 0x0003); // NOLINT

  // clean up
  cpu_write_mem(&cpu, 0x0001, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0x00); // NOLINT
}

void
test_opcode_0x1a(void) // NOLINT
{                      // LDAX D

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu_write_mem(&cpu, 0x0001, 0xCC); // NOLINT
  cpu.d = 0x00;                      // NOLINT
  cpu.e = 0x01;                      // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0x1a); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0xCC == cpu.a); // NOLINT

  // cleanup
  cpu_write_mem(&cpu, 0x0001, 0x00); // NOLINT
}

void
test_opcode_0x23(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  int code_found = execute_instruction(&cpu, 0x23); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu.l == 1);
  CU_ASSERT(cpu.h == 0);

  cpu.l = 0xFF;                                 // NOLINT
  code_found = execute_instruction(&cpu, 0x23); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 2);
  CU_ASSERT(cpu.l == 0);
  CU_ASSERT(cpu.h == 1);
}

void
test_opcode_0x29(void) // NOLINT
{                      // DAD H

  /* no carry test */
  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu.h = 0x11; // NOLINT
  cpu.l = 0xaa; // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0x29); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0x23 == cpu.h); // NOLINT
  CU_ASSERT(0x54 == cpu.l); // NOLINT
  CU_ASSERT(FLAG_CY != (cpu.flags & FLAG_CY));

  /* carry test */
  // setup
  cpu.h = 0x80; // NOLINT
  cpu.l = 0x00; // NOLINT

  // execute
  code_found = execute_instruction(&cpu, 0x29); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(2 == cpu.pc);
  CU_ASSERT(0x00 == cpu.h); // NOLINT
  CU_ASSERT(0x00 == cpu.l); // NOLINT
  CU_ASSERT(FLAG_CY == (cpu.flags & FLAG_CY));
}

void
test_opcode_0x32(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu_write_mem(&cpu, 0x0001, 0xAA); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0xBB); // NOLINT

  cpu.a = 0x11; // NOLINT

  int code_found = execute_instruction(&cpu, 0x32); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 3);
  CU_ASSERT(cpu_read_mem(&cpu, 0xBBAA) == cpu.a);

  // clean up
  cpu_write_mem(&cpu, 0x0001, 0x00);
  cpu_write_mem(&cpu, 0x0002, 0x00);
}

void
test_opcode_0x35(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);
  cpu.h = 0xAA; // NOLINT
  cpu.l = 0xBB; // NOLINT

  cpu_write_mem(&cpu, 0xAABB, 0x02); // NOLINT

  // decrease (HL) from 2 to 1
  int code_found = execute_instruction(&cpu, 0x35); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu_read_mem(&cpu, 0xAABB) == 0x01); // NOLINT
  CU_ASSERT((cpu.flags & FLAG_P) == 0);
  CU_ASSERT((cpu.flags & FLAG_S) == 0);
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);
  CU_ASSERT((cpu.flags & FLAG_AC) == FLAG_AC);

  // decrease (HL) from 1 to 0
  code_found = execute_instruction(&cpu, 0x35); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 2);
  CU_ASSERT(cpu_read_mem(&cpu, 0xAABB) == 0x00);
  CU_ASSERT((cpu.flags & FLAG_P) == FLAG_P);
  CU_ASSERT((cpu.flags & FLAG_S) == 0);
  CU_ASSERT((cpu.flags & FLAG_Z) == FLAG_Z);
  CU_ASSERT((cpu.flags & FLAG_AC) == FLAG_AC);

  // decrease C from 0 to 255
  code_found = execute_instruction(&cpu, 0x35); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 3);
  CU_ASSERT(cpu_read_mem(&cpu, 0xAABB) == 0xFF);
  CU_ASSERT((cpu.flags & FLAG_P) == FLAG_P);
  CU_ASSERT((cpu.flags & FLAG_S) == FLAG_S);
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);
  CU_ASSERT((cpu.flags & FLAG_AC) == 0);

  // clean up
  cpu_write_mem(&cpu, 0xAABB, 0x00); // NOLINT
}

void
test_opcode_0x3a(void) // NOLINT
{                      // LDA adr

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu_write_mem(&cpu, 0x0001, 0xAB); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0xCD); // NOLINT
  cpu_write_mem(&cpu, 0xcdab, 0x42); // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0x3a); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(3 == cpu.pc);
  CU_ASSERT(0x42 == cpu.a); // NOLINT

  // cleanup
  cpu_write_mem(&cpu, 0x0001, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0xcdab, 0x00); // NOLINT
}

void
test_opcode_0x66(void) // NOLINT
{                      // MOV H,M

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu_write_mem(&cpu, 0x1234, 0x31); // NOLINT
  cpu.h = 0x12;                      // NOLINT
  cpu.l = 0x34;                      // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0x66); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0x31 == cpu.h); // NOLINT

  // cleanup
  cpu_write_mem(&cpu, 0x1234, 0x00); // NOLINT
}

void
test_opcode_0x7b(void) // NOLINT
{                      // MOV A,E

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu.e = 0x29; // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0x7b); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0x29 == cpu.a); // NOLINT
  CU_ASSERT(0x29 == cpu.e); // NOLINT
}

void
test_opcode_0xaf(void) // NOLINT
{                      // XRA A

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu.a = 0xcc; // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0xaf); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0x00 == cpu.a); // NOLINT
  CU_ASSERT(FLAG_S != (cpu.flags & FLAG_S));
  CU_ASSERT(FLAG_Z == (cpu.flags & FLAG_Z));
  CU_ASSERT(FLAG_P == (cpu.flags & FLAG_P));
  CU_ASSERT(FLAG_CY != (cpu.flags & FLAG_CY));
  CU_ASSERT(FLAG_AC != (cpu.flags & FLAG_AC));
}

void
test_opcode_0xc5(void) // NOLINT
{                      // PUSH B

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu.b = 0xcd;    // NOLINT
  cpu.c = 0xab;    // NOLINT
  cpu.sp = 0xcccc; // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0xc5); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0xccca == cpu.sp); // NOLINT
  CU_ASSERT(cpu.b == cpu_read_mem(&cpu, cpu.sp + 1));
  CU_ASSERT(cpu.c == cpu_read_mem(&cpu, cpu.sp));

  // cleanup
  cpu_write_mem(&cpu, cpu.sp, 0x00);
  cpu_write_mem(&cpu, cpu.sp + 1, 0x00);
  CU_ASSERT(0 == cpu_read_mem(&cpu, cpu.sp + 1));
  CU_ASSERT(0 == cpu_read_mem(&cpu, cpu.sp));
}

void
test_opcode_0xd1(void) // NOLINT
{                      // POP D

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu.sp = 0xccca;                       // NOLINT
  cpu_write_mem(&cpu, cpu.sp, 0x23);     // NOLINT
  cpu_write_mem(&cpu, cpu.sp + 1, 0xcd); // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0xd1); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0xcccc == cpu.sp); // NOLINT
  CU_ASSERT(0xcd == cpu.d);    // NOLINT
  CU_ASSERT(0x23 == cpu.e);    // NOLINT

  // cleanup
  cpu_write_mem(&cpu, cpu.sp, 0x00);     // NOLINT
  cpu_write_mem(&cpu, cpu.sp + 1, 0x00); // NOLINT
}

void
test_opcode_0xda(void) // NOLINT
{
  // JMP CY
  i8080 cpu;
  cpu_init(&cpu);

  cpu.pc = 0x1234;                       // NOLINT
  cpu.flags = 0;                         // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 1, 0xBB); // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 2, 0xAA); // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 4, 0xDD); // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 5, 0xCC); // NOLINT

  int code_found = execute_instruction(&cpu, 0xda); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT_EQUAL(cpu.pc, 0x1237); // NOLINT

  cpu.flags |= FLAG_CY;
  code_found = execute_instruction(&cpu, 0xda); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT_EQUAL(cpu.pc, 0xCCDD); // NOLINT

  cpu_write_mem(&cpu, 0x1235, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0x1236, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0x1237, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0x1238, 0x00); // NOLINT
}

void
test_opcode_0xe5(void) // NOLINT
{                      // PUSH H

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu.h = 0x12;    // NOLINT
  cpu.l = 0x56;    // NOLINT
  cpu.sp = 0xddbb; // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0xe5); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0xddb9 == cpu.sp); // NOLINT
  CU_ASSERT(cpu.h == cpu_read_mem(&cpu, cpu.sp + 1));
  CU_ASSERT(cpu.l == cpu_read_mem(&cpu, cpu.sp));

  // cleanup
  cpu_write_mem(&cpu, cpu.sp, 0x00);     // NOLINT
  cpu_write_mem(&cpu, cpu.sp + 1, 0x00); // NOLINT
}

void
test_opcode_0xf5(void) // NOLINT
{                      // PUSH PSW

  // setup
  i8080 cpu;
  cpu_init(&cpu);
  cpu.a = 0x2a;                   // NOLINT
  cpu.flags = (FLAG_P | FLAG_CY); // NOLINT
  cpu.sp = 0x4444;                // NOLINT

  // execute
  int code_found = execute_instruction(&cpu, 0xf5); // NOLINT

  // verify
  CU_ASSERT(0 == code_found);
  CU_ASSERT(1 == cpu.pc);
  CU_ASSERT(0x4442 == cpu.sp); // NOLINT
  CU_ASSERT(cpu.a == cpu_read_mem(&cpu, cpu.sp + 1));
  CU_ASSERT(cpu.flags == cpu_read_mem(&cpu, cpu.sp));

  // cleanup
  cpu_write_mem(&cpu, cpu.sp, 0x00);     // NOLINT
  cpu_write_mem(&cpu, cpu.sp + 1, 0x00); // NOLINT
}

void
test_opcode_0x56(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu_write_mem(&cpu, 0xBBAA, 0x11); // NOLINT

  cpu.h = 0xBB; // NOLINT
  cpu.l = 0xAA; // NOLINT

  int code_found = execute_instruction(&cpu, 0x56); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu.d == cpu_read_mem(&cpu, 0xBBAA));

  // clean up
  cpu_write_mem(&cpu, 0xBBAA, 0x00); // NOLINT
}

void
test_opcode_0x77(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu.a = 0x11; // NOLINT

  cpu.h = 0xBB; // NOLINT
  cpu.l = 0xAA; // NOLINT

  int code_found = execute_instruction(&cpu, 0x77); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu_read_mem(&cpu, 0xBBAA) == cpu.a);

  // clean up
  cpu_write_mem(&cpu, 0xBBAA, 0x00); // NOLINT
}

void
test_opcode_0x7e(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu_write_mem(&cpu, 0xBBAA, 0x11); // NOLINT

  cpu.h = 0xBB; // NOLINT
  cpu.l = 0xAA; // NOLINT

  int code_found = execute_instruction(&cpu, 0x7e); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 1);
  CU_ASSERT(cpu.a == cpu_read_mem(&cpu, 0xBBAA));

  // clean up
  cpu_write_mem(&cpu, 0xBBAA, 0x00); // NOLINT
}

void
test_opcode_0xc2(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu_write_mem(&cpu, 0x0001, 0xAA); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0xBB); // NOLINT

  // set flag to Zero
  update_zero_flag(&cpu, 0x00);

  int code_found = execute_instruction(&cpu, 0xc2); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 3);
  CU_ASSERT((cpu.flags & FLAG_Z) == FLAG_Z);

  // set flag to Non-Zero
  update_zero_flag(&cpu, 0x01);
  cpu.pc = 0;

  code_found = execute_instruction(&cpu, 0xc2); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 0xBBAA);
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);

  // clean up
  cpu_write_mem(&cpu, 0x0001, 0x00);
  cpu_write_mem(&cpu, 0x0002, 0x00);
}

void
test_opcode_0xc9(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  // write memory address in stack pointer
  cpu_write_mem(&cpu, 0x0001, 0xAA); // NOLINT
  cpu_write_mem(&cpu, 0x0002, 0xBB); // NOLINT

  cpu.sp = 0x0001;

  int code_found = execute_instruction(&cpu, 0xc9); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 0xBBAA);
  CU_ASSERT(cpu.sp == 0x0003);

  // clean up
  cpu_write_mem(&cpu, 0x0001, 0x00);
  cpu_write_mem(&cpu, 0x0002, 0x00);
}

void
test_opcode_0xd5(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu.d = 0xBB;    // NOLINT
  cpu.e = 0xAA;    // NOLINT
  cpu.sp = 0xFFFF; // NOLINT

  int code_found = execute_instruction(&cpu, 0xd5); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 0x0001);
  CU_ASSERT(cpu.sp == 0xFFFD);
  CU_ASSERT(cpu_read_mem(&cpu, cpu.sp) == cpu.e);
  CU_ASSERT(cpu_read_mem(&cpu, cpu.sp + 1) == cpu.d);

  // clean up
  cpu_write_mem(&cpu, 0xFFFE, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0xFFFD, 0x00); // NOLINT
}

void
test_opcode_0xeb(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu.d = 0xAA; // NOLINT
  cpu.e = 0xBB; // NOLINT
  cpu.h = 0xCC; // NOLINT
  cpu.l = 0xDD; // NOLINT

  int code_found = execute_instruction(&cpu, 0xeb); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 0x0001);
  CU_ASSERT(cpu.d == 0xCC);
  CU_ASSERT(cpu.e == 0xDD);
  CU_ASSERT(cpu.h == 0xAA);
  CU_ASSERT(cpu.l == 0xBB);
}

void
test_opcode_0xfe(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);

  cpu_write_mem(&cpu, 0x0001, 0x01); // NOLINT
  cpu_write_mem(&cpu, 0x0003, 0x02); // NOLINT
  cpu_write_mem(&cpu, 0x0005, 0x00); // NOLINT
  cpu.a = 0x01;

  int code_found = execute_instruction(&cpu, 0xfe); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 0x0002);
  CU_ASSERT((cpu.flags & FLAG_P) == FLAG_P);
  CU_ASSERT((cpu.flags & FLAG_S) == 0);
  CU_ASSERT((cpu.flags & FLAG_Z) == FLAG_Z);
  CU_ASSERT((cpu.flags & FLAG_AC) == FLAG_AC);
  CU_ASSERT((cpu.flags & FLAG_CY) == 0);

  code_found = execute_instruction(&cpu, 0xfe); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 0x0004);
  CU_ASSERT((cpu.flags & FLAG_P) == FLAG_P);
  CU_ASSERT((cpu.flags & FLAG_S) == FLAG_S);
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);
  CU_ASSERT((cpu.flags & FLAG_AC) == 0);
  CU_ASSERT((cpu.flags & FLAG_CY) == FLAG_CY);

  code_found = execute_instruction(&cpu, 0xfe); // NOLINT
  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.pc == 0x0006);
  CU_ASSERT((cpu.flags & FLAG_P) == 0);
  CU_ASSERT((cpu.flags & FLAG_S) == 0);
  CU_ASSERT((cpu.flags & FLAG_Z) == 0);
  CU_ASSERT((cpu.flags & FLAG_AC) == 0);
  CU_ASSERT((cpu.flags & FLAG_CY) == 0);

  // clean up
  cpu_write_mem(&cpu, 0x0001, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0x0003, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0x0005, 0x00); // NOLINT
}

void
test_handle_interrupt(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);
  cpu.interrupt_enabled = true;
  cpu.pc = 0xAABB; // NOLINT
  cpu.sp = 0xFFFF; // NOLINT

  int interrupt_handled = handle_interrupt(&cpu, 0x01); // NOLINT

  CU_ASSERT(interrupt_handled == 0);
  CU_ASSERT(cpu.pc == 0x0008);                       // NOLINT
  CU_ASSERT(cpu.sp == 0xFFFD);                       // NOLINT
  CU_ASSERT(cpu.interrupt_enabled == false);         // NOLINT
  CU_ASSERT(cpu_read_mem(&cpu, cpu.sp) == 0xBB);     // NOLINT
  CU_ASSERT(cpu_read_mem(&cpu, cpu.sp + 1) == 0xAA); // NOLINT

  // clean up
  cpu_write_mem(&cpu, 0xFFFE, 0x00); // NOLINT
  cpu_write_mem(&cpu, 0xFFFD, 0x00); // NOLINT
}

void
test_handle_interrupt_invalid_rst(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);
  cpu.interrupt_enabled = true;
  cpu.pc = 0xAABB; // NOLINT
  cpu.sp = 0xFFFF; // NOLINT

  int interrupt_handled = handle_interrupt(&cpu, 0x08); // NOLINT

  CU_ASSERT(interrupt_handled == -1);
  CU_ASSERT(cpu.interrupt_enabled == true);
  CU_ASSERT(cpu.pc == 0xAABB); // NOLINT
  CU_ASSERT(cpu.sp == 0xFFFF); // NOLINT
}

void
test_handle_interrupt_interrupts_disabled(void) // NOLINT
{
  i8080 cpu;
  cpu_init(&cpu);
  cpu.pc = 0xAABB; // NOLINT
  cpu.sp = 0xFFFF; // NOLINT

  int interrupt_handled = handle_interrupt(&cpu, 0x01); // NOLINT

  CU_ASSERT(interrupt_handled == 0);
  CU_ASSERT(cpu.interrupt_enabled == false);
  CU_ASSERT(cpu.pc == 0xAABB); // NOLINT
  CU_ASSERT(cpu.sp == 0xFFFF); // NOLINT
}

int
main(void)
{
  CU_pSuite pSuite = NULL;

  if (CUE_SUCCESS != CU_initialize_registry())
    {
      return CU_get_error();
    }

  pSuite = CU_add_suite("opcodes", init_opcodes_suite, clean_suite);
  if (NULL == pSuite)
    {
      CU_cleanup_registry();
      return CU_get_error();
    }

  /*
   * Add new tests to this statement.  Replace the second argument of
   * CU_add_test with an output string for running the test, and the third
   * argument with the test function name.
   */
  if ((NULL
       == CU_add_test(pSuite, "test of test_opcode_0x13()", test_opcode_0x13))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x05()",
                         test_opcode_0x05))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x0e()",
                         test_opcode_0x0e))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x19()",
                         test_opcode_0x19))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x01()",
                         test_opcode_0x01))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x00()",
                         test_opcode_0x00))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x09()",
                         test_opcode_0x09))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x11()",
                         test_opcode_0x11))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x21()",
                         test_opcode_0x21))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x31()",
                         test_opcode_0x31))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x3e()",
                         test_opcode_0x3e))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x6f()",
                         test_opcode_0x6f))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x7c()",
                         test_opcode_0x7c))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xc1()",
                         test_opcode_0xc1))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xc6()",
                         test_opcode_0xc6))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x06()",
                         test_opcode_0x06))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xd3()",
                         test_opcode_0xd3))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xdb()",
                         test_opcode_0xdb))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xe6()",
                         test_opcode_0xe6))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xfb()",
                         test_opcode_0xfb))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x0d()",
                         test_opcode_0x0d))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x13()",
                         test_opcode_0x13))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x0f()",
                         test_opcode_0x0f))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x1a()",
                         test_opcode_0x1a))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x23()",
                         test_opcode_0x23))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x29()",
                         test_opcode_0x29))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x32()",
                         test_opcode_0x32))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x35()",
                         test_opcode_0x35))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x3a()",
                         test_opcode_0x3a))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x56()",
                         test_opcode_0x56))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x66()",
                         test_opcode_0x66))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x77()",
                         test_opcode_0x77))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x7b()",
                         test_opcode_0x7b))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x7e()",
                         test_opcode_0x7e))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xaf()",
                         test_opcode_0xaf))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xc2()",
                         test_opcode_0xc2))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xc5()",
                         test_opcode_0xc5))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xc9()",
                         test_opcode_0xc9))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xd1()",
                         test_opcode_0xd1))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xd5()",
                         test_opcode_0xd5))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xda()",
                         test_opcode_0xda))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xe5()",
                         test_opcode_0xe5))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xeb()",
                         test_opcode_0xeb))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xf5()",
                         test_opcode_0xf5))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xfe()",
                         test_opcode_0xfe))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x26()",
                         test_opcode_0x26))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x36()",
                         test_opcode_0x36))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x5e()",
                         test_opcode_0x5e))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x7a()",
                         test_opcode_0x7a))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xe1()",
                         test_opcode_0xe1))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xf1()",
                         test_opcode_0xf1))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xca()",
                         test_opcode_0xca))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xcd()",
                         test_opcode_0xcd))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xc3()",
                         test_opcode_0xc3))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xa7()",
                         test_opcode_0xa7))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xc8()",
                         test_opcode_0xc8))
      || (NULL
          == CU_add_test(pSuite, "test of test_handle_interrupt()",
                         test_handle_interrupt))
      || (NULL
          == CU_add_test(pSuite, "test of test_handle_interrupt_invalid_rst()",
                         test_handle_interrupt_invalid_rst))
      || (NULL
          == CU_add_test(pSuite, "test_handle_interrupt_interrupts_disabled()",
                         test_handle_interrupt_interrupts_disabled)))
    {
      CU_cleanup_registry();
      return CU_get_error();
    }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}
