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
}

void
test_opcode_0x19(void) // NOLINT
{
  // DAD D
  i8080 cpu;
  cpu_init(&cpu);

  cpu.d = 255; // NOLINT
  cpu.h = 63;  // NOLINT
  cpu.l = 200; // NOLINT

  int code_found = execute_instruction(&cpu, 0x19); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.h == 64);
  CU_ASSERT(cpu.l == 199);
  CU_ASSERT((cpu.flags & FLAG_CY) == 0);
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
  cpu.flags = 136; // NOLINT

  int code_found = execute_instruction(&cpu, 0xa7);

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

  cpu.sp = 56030;
  cpu_write_mem(&cpu, cpu.sp, 188);
  cpu_write_mem(&cpu, cpu.sp + 1, 254);

  int code_found = execute_instruction(&cpu, 0xe1);

  CU_ASSERT(code_found == 0);
  CU_ASSERT(cpu.l == 188);
  CU_ASSERT(cpu.h == 254);
  CU_ASSERT(cpu.sp == 56032);
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

  int code_found = execute_instruction(&cpu, 0xf1);

  CU_ASSERT(code_found == 0);
  CU_ASSERT_EQUAL(cpu.flags, 0xA8); // NOLINT
  CU_ASSERT_EQUAL(cpu.a, 0xDE);     // NOLINT
}

void
test_opcode_0xcd(void) // NOLINT
{
  // Call addr
  i8080 cpu;
  cpu_init(&cpu);

  cpu.pc = 0xABCD;                       // NOLINT
  cpu.sp = 0xDCBA;                       // NOLINT
  int16_t temp = cpu.sp;                 // Holds original sp addr
  cpu_write_mem(&cpu, cpu.pc + 1, 0xFE); // NOLINT
  cpu_write_mem(&cpu, cpu.pc + 2, 0xEF); // NOLINT

  int code_found = execute_instruction(&cpu, 0xcd); // NOLINT

  CU_ASSERT(code_found == 0);
  CU_ASSERT_EQUAL(cpu_read_mem(&cpu, temp - 1), 0xAB); // NOLINT
  CU_ASSERT_EQUAL(cpu_read_mem(&cpu, temp - 2), 0xCD); // NOLINT
  CU_ASSERT_EQUAL(cpu.sp, 0xDCBA - 2);                 // NOLINT
  CU_ASSERT_EQUAL(cpu.pc, 0xEFFE);                     // NOLINT
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
}

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
          == CU_add_test(pSuite, "test of test_opcode_0x0d()",
                         test_opcode_0x0d))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x13()",
                         test_opcode_0x13))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x23()",
                         test_opcode_0x23))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x32()",
                         test_opcode_0x32))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x56()",
                         test_opcode_0x56))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x77()",
                         test_opcode_0x77))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0x7e()",
                         test_opcode_0x7e))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xc2()",
                         test_opcode_0xc2))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xc9()",
                         test_opcode_0xc9))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xd5()",
                         test_opcode_0xd5))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xeb()",
                         test_opcode_0xeb))
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
          == CU_add_test(pSuite, "test of test_opcode_0xcd()",
                         test_opcode_0xcd))
      || (NULL
          == CU_add_test(pSuite, "test of test_opcode_0xc3()",
                         test_opcode_0xc3)))
    {
      CU_cleanup_registry();
      return CU_get_error();
    }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}
