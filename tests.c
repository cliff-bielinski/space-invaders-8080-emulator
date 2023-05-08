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
  CU_ASSERT(0xCC == cpu.d); // NOLINT

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
       == CU_add_test(pSuite, "test of test_opcode_0x01()", test_opcode_0x01))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x06()",
                         test_opcode_0x06))
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
                         test_opcode_0xfe)))
    {
      CU_cleanup_registry();
      return CU_get_error();
    }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}
